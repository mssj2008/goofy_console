#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_SSD1306 Display(108,64,&Wire);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BLOCKL 8
#define HBLOCK BLOCKL/2
#define MAXX SCREEN_WIDTH / BLOCKL - 1
#define MAXY SCREEN_HEIGHT / BLOCKL - 1
#define AREA (MAXX+1)*(MAXY+1) // number of blocks on the screen
#define TickInterval 10 // * 10ms

struct Point
{
  int x;
  int y;
};

struct Queue
{
  char Q[AREA];
  int16_t front=0;
  int16_t back=0;
  int16_t size=0;
};

struct Snake
{
  struct Point front;
  char LastDir; // to prevent going back into the snake(instant U turn prevention)
  struct Point frontdir;
  struct Point back;
  struct Queue Moves;
};

struct Point UP;
struct Point DOWN;
struct Point LEFT;
struct Point RIGHT;

int Score = 0;
struct Snake Player;
struct Point Food;
struct Point foodpositions[5];

void Set_Point(struct Point &p, int valx, int valy)
{
  p.x = valx;
  p.y = valy;
}
void Set_Point(struct Point &p, struct Point &p2)
{
  p.x = p2.x;
  p.y = p2.y;
}
//Blocks in the game... not points rly(too lazy to change the names)
void Draw_Point(struct Point &p,bool circle = false)
{
  if(!circle)
    Display.fillRect(p.x * BLOCKL, p.y * BLOCKL, BLOCKL, BLOCKL, SSD1306_WHITE);
  else
    Display.fillCircle(p.x * BLOCKL + HBLOCK, p.y * BLOCKL + HBLOCK,HBLOCK-1,SSD1306_WHITE);
  Display.display();
}
void Clear_Point(struct Point &p)
{
  Display.fillRect(p.x * BLOCKL, p.y * BLOCKL, BLOCKL, BLOCKL, SSD1306_BLACK);
  Display.display();
}
void Move_Point(struct Point &p, struct Point &dir);
struct Point Predict_Point(struct Point &p, struct Point &dir);
bool Get_Point(struct Point &p)
{
  return Display.getPixel(p.x * BLOCKL,p.y * BLOCKL);
}
char Get_Dir(struct Point &dir)
{
  if(dir.x == 1)
    return 'R';
  if(dir.x == -1)
    return 'L';
  if(dir.y == 1)
    return 'D';
  if(dir.y == -1)
    return 'U';
  else
    return 'X';
}
bool Match_Point(struct Point &p1, struct Point &p2)
{
  return p1.x == p2.x && p1.y == p2.y;
}
void Print_Point(struct Point &p,char end = '\n')
{
  Serial.print("X : ");
  Serial.print(p.x);
  Serial.print(" Y : ");
  Serial.print(p.y);
  Serial.print(end);
}

void QPush(struct Queue &q,char c)
{
  if(q.size >= AREA)
    return;
  q.Q[q.back] = c;
  q.size++;
  q.back--;
  if(q.back < 0)
  q.back = AREA-1;
}
char QPop(struct Queue &q)
{
  if(q.size == 0)
    return ' ';
  char c = q.Q[q.front];
  q.size--;
  q.front--;
  if(q.front < 0)
    q.front = AREA-1;
  return c;
}

void game_init();
void Spawn_Food();
void Move_Player();
void Input_Update();

void setup()
{ 
  Serial.begin(9600);
  Set_Point(UP, 0, -1);
  Set_Point(DOWN, 0, 1);
  Set_Point(RIGHT, 1, 0);
  Set_Point(LEFT, -1, 0);
  // put your setup code here, to run once:
  if (!Display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("Display Failed!");
    while (true);
  }
  Serial.println("Display Success");
  Display.display();
  delay(2000);
  Display.clearDisplay();
  Display.fillRect(20, 20, 20, 20, SSD1306_WHITE);
  Display.display();
  game_init();
}

void loop()
{
  int cnt = 0;
  // put your main code here, to run repeatedly:
  while (true) // game status
  {
    if(cnt == TickInterval) // Game Tick
    {
    cnt = 0;
    Move_Player();
    }
    delay(10);
    Input_Update();
    cnt++;
  }
}

void Move_Point(struct Point &p, struct Point &dir)
{
  int newx = p.x + dir.x;
  int newy = p.y + dir.y;
  if (newx > MAXX)
    newx = 0;
  else if (newx < 0)
    newx = MAXX;
  if (newy > MAXY)
    newy = 0;
  else if (newy < 0)
    newy = MAXY;
  Set_Point(p, newx, newy);
}

struct Point Predict_Point(struct Point p,struct Point &dir)
{
  Set_Point(p,p.x + dir.x,p.y + dir.y);
  return p;
}

void game_init()
{
  Score = 0;
  Display.clearDisplay();
  Display.display();
  QPush(Player.Moves,'L');
  Player.LastDir = 'L';
  Set_Point(Player.front, (SCREEN_WIDTH / BLOCKL) / 2 - 1, (SCREEN_HEIGHT / BLOCKL) / 2 - 1);
  Set_Point(Player.back, (SCREEN_WIDTH / BLOCKL) / 2, (SCREEN_HEIGHT / BLOCKL) / 2 - 1);
  //Set_Point(Food,100,100);
  Set_Point(Food,(SCREEN_WIDTH / BLOCKL) / 2-4, (SCREEN_HEIGHT / BLOCKL) / 2 - 1);
  Draw_Point(Food,true);
  Draw_Point(Player.front);
  Draw_Point(Player.back);
  Set_Point(Player.frontdir, -1, 0);
  delay(500 + TickInterval);
}

void Move_Player()
{
  Move_Point(Player.front, Player.frontdir);
  Player.LastDir = Get_Dir(Player.frontdir);
  
  QPush(Player.Moves,Get_Dir(Player.frontdir));
  //Serial.print(Player.Moves.Q[Player.Moves.back+1]);
  //Serial.println(Player.Moves.Q[Player.Moves.front]);
  Draw_Point(Player.front);
  if(!Match_Point(Player.front,Food))
  {
  Clear_Point(Player.back);
  //Find the Next Snake Block after back from the Move Pattern
  char c = QPop(Player.Moves);
  switch (c)
  {
    case 'L':
      Move_Point(Player.back,LEFT);
      break;

    case 'R':
      Move_Point(Player.back,RIGHT);
      break;

    case 'U':
      Move_Point(Player.back,UP);
      break;

    case 'D':
      Move_Point(Player.back,DOWN);
      break;

    default:
      Serial.println("damm");
      break;
  }
  //Serial.println(c);
  //Serial.println(Player.Moves.size);
  //Print_Point(Player.front,' ');
  //Print_Point(Player.back);
  }
  else
  {
    Serial.println("xom nom");
    Spawn_Food();
  }
}

void Spawn_Food()
{
  const uint16_t points = AREA-Player.Moves.size-1;
  
  
  uint16_t count = 0;
  //Serial.println("Checking...");
  //Serial.println(points);
  //delay(500);
  for(int i = 0; i <= MAXY; i++)
  {
    for(int j = 0; j <= MAXX; j++)
    {
      if(!Display.getPixel(j*BLOCKL,i*BLOCKL))
      {
      Serial.write('O');
      Set_Point(foodpositions[count],j,i);
      /*Set_Point(foodpositions[count],i,j);
      Serial.print("Checking X : ");
      Serial.print(foodpositions[count].x);
      Serial.print("  Y : ");
      Serial.print(foodpositions[count].y);
      Serial.println();
      delay(20);*/
        count++;
      }
      else
      {
      Serial.write('X');
      
      }
    }
    Serial.println();
  }
  //Serial.println("Check done");
  ///Serial.println(count);
  //delay(1000);
  count = random()%points;
  Serial.println(points);
  Serial.println(count);
  Set_Point(Food,foodpositions[count]);
  Draw_Point(Food,true);
  Serial.print("new food X : ");
  Serial.print(Food.x);
  Serial.print("  Y : ");
  Serial.print(Food.y);
  Serial.println();
  
  //delay(500);
}

void Input_Update()
{
  int Xval = analogRead(15);
  int Yval = analogRead(14);
  Xval -= 512; // ADC reads 850 max instead of 1024 (850/2 = 425)
  Yval -= 512;
  // normalize
  float Jx = (float)Xval / 512;
  float Jy = (float)Yval / 512;

  // Input Handling
  if (abs(Jx) < 0.75 && abs(Jy) < 0.75)
    return;
  if (abs(Jx) > 0.75 && abs(Jy) > 0.75)
    return;
  char dir = Player.LastDir; // prevent goign the oppsite direction(Back into the snake)
  if (abs(Jx) > 0.5)
  {
    if (Jx < 0 && dir != 'R')
    {
      Set_Point(Player.frontdir, LEFT);
    }
    else if (Jx > 0 && dir != 'L')
    {
      Set_Point(Player.frontdir, RIGHT);
    }
  }
  else
  {
    if (Jy < 0 && dir != 'D')
    {
      Set_Point(Player.frontdir, UP);
    }
    else if(Jy > 0 && dir != 'U')
    {
      Set_Point(Player.frontdir, DOWN);
    }
  }
  //Player.Moves.Q[Player.Moves.front] = Get_Dir(Player.frontdir);
}