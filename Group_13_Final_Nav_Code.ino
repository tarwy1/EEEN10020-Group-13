/*
 * This is the current working file for our (Group 13) Robot
 * The features that have currently been implemented are a method of storing the
 * locations of everything on the board in structures, and two functions which allow
 * for a target heading to be selected, and relevant calculations performed to find how much and which way to turn
 */

// including relevant libraries for the lcd/9-axis sensor
// these are subject to change, will need servo.h and the LCD's main library in future
#include <Wire.h>
#include <Arduino_NineAxesMotion.h>

/*
 * Below here is where the map is stored in code.
 * A basic explanation of the data structures is provided in diagrams in the 
 * main brainstorming document.
 * All of the positions in this are considering to south scoring zone to be the bottom left of the board.
 */

// struct used for a ball, containing all relevant values
struct ball{
  int color; // 0=white 1=yellow 2=red 3=multi
  float value; // 1/2=white 2=yellow 4=red 7=multi
  int pos[2]; // posX, posY
  float radius; // in mm (used to infer if the robot has captured a ball)
};

// structure used for general rectangles, scoring and starting zones mainly, but also the bumps, which can be considered rectangles
// the posX and posY are based on the bottom left corner of the rectangle
struct rect{
  int dim[4]; // posX posY lenX lenY (bottom left corner)
};

// the main board structure, of which there will be 1 instance
// has arrays containing the balls, bumps, and our scoring/start zones
// this is subject to change, will likely add the enemy start/score zones to this
struct Board{
  ball* Balls; // balls
  rect* bumps; // bumps
  rect scoring; // our score zone
  rect starting; // our start zone
  int dim[2]; // lenx leny (overall dimensions of the board)
};

// array used to store current values from the 9 axis sensor
float LiveValues[3] = {0.0f, 0.0f, 0.0f}; // heading, accelX, accelY

// this is a constant which is subject to change
// the motor power value will be adjusted while in the turn using a formula 
// in the form a * x^2 with x being the number of degrees from the current heading to the target
// this allows for the robot to slow its turn as it gets closer to the desired heading
const float motorAngleConst = 1/341.05;

// creating an instance of the board structure
Board board;

// creating an instance of the 9axis sensor
NineAxesMotion mySensor;

void setup() {
  // initialiser to fill all the arrays, values etc of the board struct.
  initBoard();

  // begin serial comms for debugging
  Serial.begin(9600);

  // begin i2c for the sensor
  I2C.begin();

  // initialise the sensor
  mySensor.initSensor();
  mySensor.setOperationMode(OPERATION_MODE_NDOF);
  mySensor.setUpdateMode(AUTO);

  // 5 second delay i swear i had this here for debugging...
  // forget exactly why its there :D
  delay(5000);
}

void loop() {
  // this is all fully subject to change
  delay(1000);
  LiveValues[0] = mySensor.readEulerHeading();
  LiveValues[1] = mySensor.readLinearAccelX();
  LiveValues[2] = mySensor.readLinearAccelY();
}

// function to populate all the values in the board struct
void initBoard(){
  // set the board dimensions
  board.dim[0] = 2440;
  board.dim[1] = 1225;

  // allocate memory for the array of balls and bumps
  board.Balls = (ball*)malloc(7 * sizeof(ball));
  board.bumps = (rect*)malloc(4 * sizeof(rect));

  // create rectangles for each bump and the scoring/starting zones
  makeRect(0, 0, 300, 300, &board.scoring);
  makeRect(1220, 0, 900, 220, &board.starting);
  makeRect(0, 300, 300, 14, &board.bumps[0]);
  makeRect(300, 0, 14, 1225, &board.bumps[1]);
  makeRect(2140, 300, 300, 14, &board.bumps[2]);
  makeRect(2140, 0, 14, 1225, &board.bumps[3]);

  // create ball structs for each of the balls 
  makeBall(605, 300, 1, 2, &board.Balls[0]); // bottom left yellow 
  makeBall(1220,399, 2, 4, &board.Balls[1]); // bottom red
  makeBall(1220,826, 2, 4, &board.Balls[2]); // top red
  makeBall(1835,925, 1, 2, &board.Balls[3]); // top right yellow
  makeBall(1220,613, 3, 7, &board.Balls[4]); // middle
  makeBall(1024,925, 1, 2, &board.Balls[5]); // middle left yellow 
  makeBall(1416,925, 1, 2, &board.Balls[6]); // middle right yellow
}

/*  this was just a quick-look info i used while writing the above code, so i could glance at the coordinates of different things
 *  
 *  bottom left yellow 605, 300
 *  bottom red 1220, 399
 *  top red 1220 826
 *  top right yellow 1835, 925
 *  middle 1220, 613
 *  middle left yellow 1024, 925
 *  middle right yellow 1416, 925
 *  
 *  int color; // 0=white 1=yellow 2=red 3=multi    
 *  int value; // 1/2=white 2=yellow 4=red 7=multi
 */

// "constructor" to fill up a ball struct from values (allows it to be done in 1 line rather than 4 :) )
void makeBall(int posx, int posy, int color, int value, struct ball* Ball){
  Ball->pos[0] = posx;
  Ball->pos[1] = posy;
  Ball->color = color;
  Ball->value = value;
}

// "constructor" to fill up a rect struct from values
void makeRect(int posx, int posy, int dimx, int dimy, rect* Rect){
  Rect->dim[0] = posx;
  Rect->dim[1] = posy;
  Rect->dim[2] = dimx;
  Rect->dim[3] = dimy;
}

// This is a function which will not be changed
// its purpose is to take the current heading and a target heading and return the number of degrees and direction to turn
// this is a complex task because heading wraps around from 360 to 0, so sometimes the "shortest" path from one heading to another
// is to go through 360/0 degrees
int degToHDG(float HDG, float target){
  float deg = target-HDG; // positive is clockwise
  if(abs(deg) >= 180){
    deg = (-abs(deg) + 360) * ((deg<0) - (deg>0));
  }
  return deg;
}

// this function is the one referenced on line 50.
// its intended to take the number of degrees to turn, and adjust the motor analogWrite 
// value with a formula so that the turn slows as it approaches the target heading.
void turn(float Deg){
  // need motor values between 160 and 255
  int motorPowerValue = (int)(motorAngleConst * Deg * Deg);
}
