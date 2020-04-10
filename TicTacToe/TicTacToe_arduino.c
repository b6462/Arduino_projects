#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

#define joyX A0
#define joyY A1

LiquidCrystal_I2C lcd(0x3f, 16, 2);

//自定义字符
byte O_[8] = { 0x0E,0x11,0x11,0x0E,0x00,0x00,0x00,0x00 };
byte OX[8] = { 0x0E,0x11,0x11,0x0E,0x00,0x0E,0x0E,0x00 };
byte OO[8] = { 0x0E,0x11,0x11,0x0E,0x0E,0x11,0x11,0x0E };
byte X_[8] = { 0x00,0x0E,0x0E,0x00,0x00,0x00,0x00,0x00 };
byte XO[8] = { 0x00,0x0E,0x0E,0x00,0x0E,0x11,0x11,0x0E };
byte XX[8] = { 0x00,0x0E,0x0E,0x00,0x00,0x0E,0x0E,0x00 };
byte _X[8] = { 0x00,0x00,0x00,0x00,0x00,0x0E,0x0E,0x00 };
byte _O[8] = { 0x00,0x00,0x00,0x00,0x0E,0x11,0x11,0x0E };
//byte __[8]={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};  NULL被逻辑运算占位 但是不能再定义空位因为内存只允许存储八个

int plate[2][3] = { 7 };  //棋盘设置初始化
int plate_r[2][3] = { 7 };  //棋盘备份 用于在不确认之后恢复棋盘
int check_plate[2][3] = { 0 };  //用于检查当前位是否为空位 为空位则保持plate 并且在该位显示空字符
int check_plate_r[2][3] = { 0 };  //检查备份
bool changed = false; //判断棋盘是否有变化
int button = 13;  //按键位置定义
int x_value = 0;
int y_value = 0;
int p[6]; //胜负判断一维数组 需要将棋盘遍历重新写入
int pace = 0;  //落子次数 大于等于5之后才进行胜负判断 免去空白时因为残留值而自动判断胜利
int x_score = 0;  //计分
int o_score = 0;

bool role = true; //落子方判断 true为圆圈false为点

void setup()
{
  memset(plate,7,sizeof(plate));  //清空
  save();
  memset(check_plate,0,sizeof(check_plate));
  save_c();
  changed = false;
  pace = 0;
  lcd.init();                  //初始化LCD
  lcd.backlight();             //打开背光
  lcd.createChar(0, O_);       //创建自定义字符与对应内存
  lcd.createChar(1, OX);
  lcd.createChar(2, OO);
  lcd.createChar(3, X_);
  lcd.createChar(4, XO);
  lcd.createChar(5, XX);
  lcd.createChar(6, _X);
  lcd.createChar(7, _O);
  pinMode(button, INPUT);
  digitalWrite(button, HIGH);
  Serial.begin(9600);
  lcd.setCursor(4,0);           //设置UI界面
  lcd.print("Player:       ");
  lcd.setCursor(4,1);
  lcd.print("Score:");
  lcd.setCursor(10,1);
  lcd.write(byte(0));
  lcd.setCursor(13,1);
  lcd.write(byte(3));
  lcd.setCursor(11,1);
  lcd.print(o_score);
  lcd.setCursor(14,1);
  lcd.print(x_score);
  
  
}

void set_board()  //根据plate遍历刷新棋盘与UI 如果check_plate显示为空值则置空字符
{
  for (int i = 0; i <= 1; i++)
  {
    for (int j = 0; j <= 2; j++)
    {
      lcd.setCursor(j, i);
      if (check_plate[i][j] == 0)
        lcd.print(" ");
      else
        lcd.write(byte(plate[i][j]));
    }
  }
  lcd.setCursor(11,1);
  lcd.print(o_score);
  lcd.setCursor(14,1);
  lcd.print(x_score);
}

void judge_role() //落子方判断
{
  if (changed)
  {
    role = !role;
    pace++;
  }
    
  lcd.setCursor(11,0);  //显示当前落子方
  if(role)
  lcd.write(byte(0));
  else
  lcd.write(byte(3));
  changed = false;
}

void save() //保存棋盘现有状态
{
  for (int i = 0; i <= 1; i++)
  {
    for (int j = 0; j <= 2; j++)
    {
      plate_r[i][j] = plate[i][j];
    }
  }
}

void load() //读取保存的棋盘状态
{
  for (int i = 0; i <= 1; i++)
  {
    for (int j = 0; j <= 2; j++)
    {
      plate[i][j] = plate_r[i][j];
    }
  }
}

void save_c()  //保存空置检测数组状态
{
  for (int i = 0; i <= 1; i++)
  {
    for (int j = 0; j <= 2; j++)
    {
      check_plate_r[i][j] = check_plate[i][j];
    }
  }
}

void load_c()  //读取空置检测数组状态
{
  for (int i = 0; i <= 1; i++)
  {
    for (int j = 0; j <= 2; j++)
    {
      check_plate[i][j] = check_plate_r[i][j];
    }
  }
}

//此函数只在可以落子地方才会被激活 所以不会覆盖已有棋子
void check_button()  //判断13号按键口是否按下
{
  set_board(); //预览落子
  if (digitalRead(button) == LOW)  //按钮是否被按下
  {
    save();//更新保存数据
    save_c();
    changed = true; //棋盘发生变化
  }
  load(); //调用保存的数据
  load_c(); 
}

int judge_main()  //胜负判断函数
{
  for(int i = 0; i <= 1; i++)
  {
    for(int j = 0; j <= 2; j++)
    {
      p[(i*3)+j] = plate[i][j]; //将二维棋盘数组转为一维数组
    }
  }
  if(pace>=5)   //只有第五步之后才有可能出现胜负
  {
  if(
  (p[0]==2&&p[3]==0)
  ||(p[1]==2&&p[4]==0)
  ||(p[2]==2&&p[5]==0)
  ||((p[0]==0||p[0]==2||p[0]==1)&&(p[1]==0||p[1]==2||p[1]==1)&&(p[2]==0||p[2]==2||p[2]==1))
  ||((p[0]==7||p[0]==2||p[0]==4)&&(p[1]==7||p[1]==2||p[1]==4)&&(p[2]==7||p[2]==2||p[2]==4))
  ||(p[3]==0&&p[4]==0&&p[5]==0)
  ||((p[1]==7||p[1]==2||p[1]==4)&&(p[2]==0||p[2]==2||p[2]==1)&&p[3]==0)
  ||((p[1]==7||p[1]==2||p[1]==4)&&(p[0]==0||p[0]==2||p[0]==1)&&p[5]==0)
  )
  {
    return 1;//O赢 
    Serial.print("O_wins");
  }
  
  else if(
  (p[0]==5&&p[3]==3)
  ||(p[1]==5&&p[4]==3)
  ||(p[2]==5&&p[5]==3)
  ||((p[0]==3||p[0]==5||p[0]==4)&&(p[1]==3||p[1]==5||p[1]==4)&&(p[2]==3||p[2]==5||p[2]==4))
  ||((p[0]==6||p[0]==5||p[0]==1)&&(p[1]==6||p[1]==5||p[1]==1)&&(p[2]==6||p[2]==5||p[2]==1))
  ||(p[3]==3&&p[4]==3&&p[5]==3)
  ||((p[1]==6||p[1]==5||p[1]==1)&&(p[2]==3||p[2]==5||p[2]==4)&&p[3]==3)
  ||((p[1]==6||p[1]==5||p[1]==1)&&(p[0]==3||p[0]==5||p[0]==4)&&p[5]==3)
  )
  {
    return 2;//X赢 
    Serial.print("X_wins");
  }
  else return 0;//没人赢 
  }
  else
  return 0;  //在第五步之前都只返回0
}

void cal(int jug)   //计分
{
  pace = 0;    //计分则说明一局结束 步数置零
  if(jug == 1)
  o_score++;
  else if(jug == 2)
  x_score++;
  set_board();
  int n = 1;   //用于延迟刷新以展示胜利界面
  /*
  for(int s = 0; s<=5; s++)
  {
    Serial.print(p[s]);
    Serial.print(" ");
  }
  Serial.print("\n");
  for(int i = 0; i <= 1; i++)
  {
    for(int j = 0; j <= 2; j++)
    {
      Serial.print(plate[i][j]);
      Serial.print(" ");
    }
    Serial.print("\n");
  }
  Serial.print("\n");
  */
  while(n)
  {
    if(digitalRead(button) == LOW)
    {
      n = 0;
      setup();
    }
  }
}

void win(int jug) //调用判断函数的输出值进行胜利判断
{
  if(jug != 0)
  {
    lcd.setCursor(4,0);
    lcd.print("Winner:");  //显示胜利者
    if(jug == 1)
    {
      lcd.write(byte(0));
      cal(jug);
      role = false;  //如果O胜利则下一局X起手
    }
    else if(jug == 2)
    {
      lcd.write(byte(3));
      cal(jug);
      role = true;  //如果X胜利则下一局O起手
    }
  }
  else if(jug == 0 && pace == 9)
  {
    lcd.setCursor(4,0);
    lcd.print("Draw      ");  //平局
    cal(jug);
  }
}

//以下是各个方位对应的判断
void up()
{
  //Serial.print("up");
  //Serial.print("\n");
  if (check_plate[0][1] == 0)//__  应当优先判断是否为空值 不然会与隐藏状态棋盘数组的状态冲突
  {
    if (role)
      plate[0][1] = 0;  //O_
    else
      plate[0][1] = 3;  //X_
    check_plate[0][1] = 1;
    check_button();
  }
  else if (plate[0][1] == 7) //_O
  {
    if (role)
      plate[0][1] = 2;  //OO
    else
      plate[0][1] = 4; //XO
    check_button();
  }
  else if (plate[0][1] == 6) //_X
  {
    if (role)
      plate[0][1] = 1;  //OX
    else
      plate[0][1] = 5;  //XX
    check_button();
  }
}


void left_up()
{
  //Serial.print("left_up");
  //Serial.print("\n");
  if (check_plate[0][0] == 0)//__
  {
    if (role)
      plate[0][0] = 0;  //O_
    else
      plate[0][0] = 3;  //X_
    check_plate[0][0] = 1;
    check_button();
  }

  else if (plate[0][0] == 7) //_O
  {
    if (role)
      plate[0][0] = 2;  //OO
    else
      plate[0][0] = 4; //XO
    check_button();
  }
  else if (plate[0][0] == 6) //_X
  {
    if (role)
      plate[0][0] = 1;  //OX
    else
      plate[0][0] = 5;  //XX
    check_button();
  }
}

void left()
{
  //Serial.print("left");
  //Serial.print("\n");
  if (check_plate[0][0] == 0)//__
  {
    if (role)
      plate[0][0] = 7;  //_O
    else
      plate[0][0] = 6;  //_X
    check_plate[0][0] = 1;
    check_button();
  }

  else if (plate[0][0] == 3) //X_
  {
    if (role)
      plate[0][0] = 4;  //XO
    else
      plate[0][0] = 5; //XX
    check_button();
  }
  else if (plate[0][0] == 0) //O_
  {
    if (role)
      plate[0][0] = 2;  //OO
    else
      plate[0][0] = 1;  //OX
    check_button();
  }
}

void left_down()
{
  //Serial.print("left_down");
  //Serial.print("\n");
  if (check_plate[1][0] == 0)//__
  {
    if (role)
      plate[1][0] = 0;  //O_
    else
      plate[1][0] = 3;  //X_
    check_plate[1][0] = 1;
    check_button();
  }
}

void down()
{
  //Serial.print("down");
  //Serial.print("\n");
  if (check_plate[1][1] == 0)//__
  {
    if (role)
      plate[1][1] = 0;  //O_
    else
      plate[1][1] = 3;  //X_
    check_plate[1][1] = 1;
    check_button();
  }
}

void right_down()
{
  //Serial.print("right_down");
  //Serial.print("\n");
  if (check_plate[1][2] == 0)//__
  {
    if (role)
      plate[1][2] = 0;  //O_
    else
      plate[1][2] = 3;  //X_
    check_plate[1][2] = 1;
    check_button();
  }
}

void right()
{
  //Serial.print("right");
  //Serial.print("\n");
  if (check_plate[0][2] == 0)//__
  {
    if (role)
      plate[0][2] = 7;  //_O
    else
      plate[0][2] = 6;  //_X
    check_plate[0][2] = 1;
    check_button();
  }

  else if (plate[0][2] == 3) //X_
  {
    if (role)
      plate[0][2] = 4;  //XO
    else
      plate[0][2] = 5; //XX
    check_button();
  }
  else if (plate[0][2] == 0) //O_
  {
    if (role)
      plate[0][2] = 2;  //OO
    else
      plate[0][2] = 1;  //OX
    check_button();
  }
}

void right_up()
{
  //Serial.print("right_up");
  //Serial.print("\n");
  if (check_plate[0][2] == 0)//__
  {
    if (role)
      plate[0][2] = 0;  //O_
    else
      plate[0][2] = 3;  //X_
    check_plate[0][2] = 1;
    check_button();
  }

  else if (plate[0][2] == 7) //_O
  {
    if (role)
      plate[0][2] = 2;  //OO
    else
      plate[0][2] = 4; //XO
    check_button();
  }
  else if (plate[0][2] == 6) //_X
  {
    if (role)
      plate[0][2] = 1;  //OX
    else
      plate[0][2] = 5;  //XX
    check_button();
  }
}

void middle()
{
  //Serial.print("middle");
  //Serial.print("\n");
  if (check_plate[0][1] == 0)//__
  {
    if (role)
      plate[0][1] = 7;  //_O
    else
      plate[0][1] = 6;  //_X
    check_plate[0][1] = 1;
    check_button();
  }

  else if (plate[0][1] == 3) //X_
  {
    if (role)
      plate[0][1] = 4;  //XO
    else
      plate[0][1] = 5; //XX
    check_button();
  }
  else if (plate[0][1] == 0) //O_
  {
    if (role)
      plate[0][1] = 2;  //OO
    else
      plate[0][1] = 1;  //OX
    check_button();
  }
}




void judge_pos() //判断摇杆方向
{
  save(); //执行备份
  save_c();
  if (x_value >= 650 && x_value <= 1030) //摇杆向左
  {
    if (y_value >= 1000 && y_value <= 1040) //左上
    {
      left_up();
    }
    else if (y_value <= 480 && y_value >= 0) //左下
    {
      left_down();
    }
    else  //正左
    {
      left();
    }
  }
  else if (x_value < +10 && x_value >= 0) //摇杆向右
  {
    if (y_value >= 930 && y_value <= 1040) //右上
    {
      right_up();
    }
    else if (y_value <= 250 && y_value >= 0) //右下
    {
      right_down();
    }
    else  //正右
    {
      right();
    }
  }
  else
  {
    if (y_value >= 1010 && y_value <= 1030) //正上
    {
      up();
    }
    else if (y_value <= 5) //正下
    {
      down();
    }
    else //正中
    {
      middle();
    }
  }
}


void loop()
{
  x_value = analogRead(joyX); //读取xy轴模拟量输入
  y_value = analogRead(joyY);
  /*
    Serial.print(x_value);
    Serial.print("  ");
    Serial.print(y_value);
    Serial.print("\n");
  */
  judge_pos();    //先判断摇杆方向 随后判断是否能够落子 进行预览 随后判断按键是否按下
  set_board();    //刷新一次棋盘
  judge_role();   //判断是否发生变化 落子方是否变化
  win(judge_main());  //判断输赢 进行计分 重置棋盘
  

  /*
  lcd.setCursor(0,0);
  lcd.write(byte(0));
  lcd.setCursor(1,0);
  lcd.write(byte(1));
  lcd.setCursor(2,0);
  lcd.write(byte(2));
  lcd.setCursor(0,1);
  lcd.write(byte(1));
  lcd.setCursor(1,1);
  lcd.write(byte(4));
  lcd.setCursor(2,1);
  lcd.write(byte(7));
  */
}
