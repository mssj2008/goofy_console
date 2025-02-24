#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

Adafruit_SSD1306 Dislpay(128, 64, &Wire, -1);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BLOCKL 8
#define MAXX SCREEN_WIDTH / BLOCKL - 1
#define MAXY SCREEN_HEIGHT / BLOCKL - 1
#define TickInterval 10 // * 10ms

struct Point
{
  int x;
  int y;
};

struct Snake
{
  struct Point front;
  char LastDir; // to prevent going back into the snake
  struct Point frontdir;
  struct Point back;
  String Moves;
};

struct Point UP;
struct Point DOWN;
struct Point LEFT;
struct Point RIGHT;

int Score = 0;
struct Snake Player;

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
void Draw_Point(struct Point &p)
{
  Dislpay.fillRect(p.x * BLOCKL, p.y * BLOCKL, BLOCKL, BLOCKL, SSD1306_WHITE);
  Dislpay.display();
}
void Clear_Point(struct Point &p)
{
  Dislpay.fillRect(p.x * BLOCKL, p.y * BLOCKL, BLOCKL, BLOCKL, SSD1306_BLACK);
  Dislpay.display();
}
void Move_Point(struct Point &p, struct Point &dir);
struct Point Predict_Point(struct Point &p, struct Point &dir);
bool Get_Point(struct Point &p)
{
  return Dislpay.getPixel(p.x * BLOCKL,p.y * BLOCKL);
}
char Get_Dir(struct Point &dir)
{
  if(dir.x == 1)
    return 'R';
  if(dir.x == -1)
    return 'L';
  if(dir.y == 1)
    return 'D';
  else
    return 'U';
}

void game_init();
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
  if (!Dislpay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("Display Failed!");
  }
  Serial.println("Display Success");
  Dislpay.display();
  delay(2000);
  Dislpay.clearDisplay();
  Dislpay.fillRect(20, 20, 20, 20, SSD1306_WHITE);
  Dislpay.display();
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
  Dislpay.clearDisplay();
  Player.Moves = "LL";
  Player.LastDir = 'L';
  Set_Point(Player.front, (SCREEN_WIDTH / BLOCKL) / 2 - 1, (SCREEN_HEIGHT / BLOCKL) / 2 - 1);
  Set_Point(Player.back, (SCREEN_WIDTH / BLOCKL) / 2, (SCREEN_HEIGHT / BLOCKL) / 2 - 1);
  Draw_Point(Player.front);
  Draw_Point(Player.back);
  Set_Point(Player.frontdir, -1, 0);
  delay(500 + TickInterval);
}

void Move_Player()
{
  Move_Point(Player.front, Player.frontdir);
  Player.LastDir = Get_Dir(Player.frontdir);
  Player.Moves.concat(Get_Dir(Player.frontdir));
  Draw_Point(Player.front);
  Clear_Point(Player.back);
  //Find the Next Snake Block after back from the Move Pattern
  switch (Player.Moves.charAt(0))
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
  }
  Player.Moves.remove(0,1);
  Serial.println(Player.Moves);
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
  Player.Moves.setCharAt(Player.Moves.length()-1,Get_Dir(Player.frontdir));
}