/* game.c - xmain, prntr */

#include <kernel.h>
#include <io.h>
#include <bios.h>
#include <conf.h>
#include <sleep.h>
#include <string.h>
#define PLANE_CENTER 40
#define LEFT_BOUND 0
#define RIGHT_BOUND 80
#define RUNWAY_START 800


extern SYSCALL  sleept(int);
extern struct intmap far *sys_imp;

	/*
		COLOR LIST - every 16 background change:
		font			background	font			background
		0-black			0
		1- dark blue		16		11- cyan	176 ?
		2- green			32		12-light red		192 ?
		3- light blue	48		13-pink				208 ?
		4- red			64		14-yellow			224 ?
		5- purple		80		15-white			240 ?
		6-brown			96	
		7-whitish		112	
		8-grey			128	?
		9- blue			144	?
		10- light green	160 ?
		*/

/*------------------------------------------------------------------------
 *  xmain  --  example of 2 processes executing the same code concurrently
 *------------------------------------------------------------------------
 */

unsigned char far *b800h=(unsigned char far*)0xb8000000;

typedef struct position
{
  int x;
  int y;

}  POSITION;
typedef enum{BLACK,DARK_BLUE,GREEN,LIGHT_BLUE,RED,PURPLE,BROWN,WHITISH,GRAY,BLUE,LIGHT_GREEN,LIGHT_TURQIZ,LIGHT_RED,PINK,YELLOW,WHITE} COLOR;
int point_in_cycle; //schedule variables
int gcycle_length;//-----//---------
int gno_of_pids;//---------//---------

int receiver_pid,update_pid,disp_pid;
char display[2001];//display serial ASCII array
char attr_disp[2001];//display serial attribute array
char ch_arr[2048];//Keyboard char buffer
int front = -1;//keyboard buffer pointers
int rear = -1;//-------//-----//--------
char display_draft[25][80];//display two dimension ASCII array
int display_att_draft[25][80];//display two dimension attribute array

//initial plane characteristics
int speed = 50;
int maxdistance=3500;//distance to the end of runway
int height=150;//height above the ground
int distance=0;//flayed distance
int v_angle = 0;
int h_angle = 0;
int gameStatus = 0;
int course = 0;
unsigned long tick = 0 ;


POSITION center;//position of the centre of the screen - relative point of drawing objects
int Up_L = -4;// runway upper left point
int Up_R =  4;//runway upper right point 
int Dw_L = -6;//runway lower left point
int Dw_R = 6;//runway lower right point
int diff;//special variable that contains Dw_R - Up_L = > provides the length of the runway
int runWayEdge;
INTPROC new_int9(int mdevno)
{
	char result = 0;
 	int scan = 0;
  	int ascii = 0;

	asm {
		 MOV AH,1
		 INT 16h
  		 JZ Skip1
 		 MOV AH,0
  		 INT 16h
  		 MOV BYTE PTR scan,AH
 		 MOV BYTE PTR ascii,AL
 	    } //asm
 	    
 	if (scan == 75)
  		 result = 'a';//LEFT
 	else
   		if (scan == 72)
			result = 'w';//UP
   	else
  		if (scan == 77)
			result = 'd';//RIGHT
	else 
	    if(scan == 80 )
	        result  = 's';
   	else 
   		if(scan==20)//speed up
   			result = 't';
  	else 
   		if(scan==34)//speed down
	  	 	 result = 'g';
   	else if ((scan == 46)&& (ascii == 3)) // Ctrl-C?
   	
	asm INT 27; // terminate xinu

	send(receiver_pid, result); 

	Skip1:
} // new_int9



void set_new_int9_newisr()
{
  int i;
  for(i=0; i < 32; i++)
    if (sys_imp[i].ivec == 9)
    {
     sys_imp[i].newisr = new_int9;
     return;
    }

} // set_new_int9_newisr

void display_Msg(int row,int column,char *msg,int data){

int i,j;
char *str_mem; 
int temp = data;
for( i=0 ;temp!=0;i++)
    temp = temp/10;
str_mem = getmem(i+1);	
sprintf(str_mem,"%d",data);
for(i=column;i<(column+strlen(msg)+1);i++){
        display_draft[row][i] = msg[i];
}
for(j=i;j<(i+strlen(str_mem)+1);j++){
    display_draft[row][j] = str_mem[j-i];
}
    
}

void gameover(char *msg,COLOR attr)
{
    int i;
	for(i=0;i<strlen(msg);i++)
	{
		display_draft[1][i+25] = msg[i];
		display_att_draft[1][i+25] = attr;
	}
}

/*------------------------------------------------------------------------
 *  prntr  --  print a character indefinitely
 *------------------------------------------------------------------------
 */

void displayer( void )
{
	int i;
	while (1)
         {
               receive();
			   for(i=0; i < 2000; i++)
				{
					b800h[i*2] = display[i];  // fill screen segment from buffer array
					b800h[i*2+1] = attr_disp[i];        //set color
				}
				if(gameStatus!=0)
				   asm INT 27;
         } //while
} // prntr


void receiver()
{
  while(1)
  {
    char temp;
    temp = receive(); 
    rear++;
	
    ch_arr[rear] = temp;
    if (front == -1)
       front = 0;
  } // while
} //  receiver

void updateter()
{
  	int i,j,dist;
  	char ch;
do{

  while(1)
  {
         // center.y++;
		  height = height -  v_angle/10 ;
		  center.x = center.x - h_angle/10;
          distance = distance + speed ;
          Dw_L = Dw_L - speed/50;
          Dw_R = Dw_R + speed/50;
		  if(v_angle!=0)
		     if(tick%4==1)
			 {
			   Up_L--;
		       Up_R++;
		       Dw_L--;
		       Dw_R++;
			   speed--;
			 }
		    
		  tick++;
   receive();
  while(front != -1)
   {
    
	 ch = ch_arr[front];
	 if(front != rear)
       front++;
     else{
		  front = -1;
		  rear = -1;
	   }
      
     
     if (((ch == 'a') || (ch == 'A'))) 
	       {
		    h_angle = h_angle - 10;
			//course = (course -1)%360;
		   }
     else if (((ch == 'd') || (ch == 'D')))
		   {
		    h_angle = h_angle + 10;
			//course = (course +1)%360;
		   }
     else if ( (ch == 'w') || (ch == 'W') )
	      {
		   v_angle = v_angle  + 10;
		   }
/////
     else if((ch=='s' || ch=='S')){
	    v_angle = v_angle - 10;
		if(v_angle < 0)
		   v_angle = 0;
	 }     
	 
	// else if ( (ch == 't') || (ch == 'T') )
	//    speed = speed + 50;
	 else if ( (ch == 'g') || (ch == 'G'))
	    speed = speed - 20;	 
////////////////////////////////////////////*/ 
      
 }// while(front != -1)
		
  // The sky
    diff = Dw_R - Up_R;
    if(diff>19)
      diff=19 ;
	for (i = 0; i < center.y; i++)
			for (j=0; j< 80; j++ )
			{
				display_draft[i][j] = ' '; //print 
				display_att_draft[i][j] = 48; // blue color 
			}
	//The ground
	for (i = center.y; i < 25; i++)
		for (j=0; j< 80; j++ )
		{
			display_draft[i][j] = ' '; //print 
			display_att_draft[i][j] = 32; // green color 
		}
        //The road init	
        dist = -1;
	for (i = center.y; i <= (center.y+diff); i++){
		dist++;
		for (j = (center.x + Up_L - dist ); j <= (center.x +Up_R + dist)&& j< 80;j++)
			{
			if(j  >= 0 ){
			if(j == center.x){
					display_draft[i][j] ='|';
					display_att_draft[i][j]= WHITE;				
					}
				else if(j==(center.x+Up_L - dist))
				{
				        display_draft[i][j] ='/';
					display_att_draft[i][j]=YELLOW;
 
				}
				else if(j==(center.x+Up_R + dist)){
				        display_draft[i][j] ='\\';
					display_att_draft[i][j]=YELLOW;
				
				}				
				else{
			            display_draft[i][j] = ' '; //print 
				    display_att_draft[i][j] = BLACK; // black color 
				} 
			  }	
			}
	}
   /*Plane body - static drawing - start - light blue BG & font*/
		for (i = 1; i < 79; i++){
			display_draft[24][i] = ' '; //print 
			display_att_draft[24][i] = 16; // color 
		}
		for (i = 5; i < 75; i++){
			display_draft[23][i] = ' '; //print 
			display_att_draft[23][i] = 16; // color 
		}
		for (i = 15; i < 65; i++){
			display_draft[22][i] = ' '; //print 
			display_att_draft[22][i] = 16; // color 
		}
		for (i = 30; i < 50; i++){
			display_draft[21][i] = ' '; //print 
			display_att_draft[21][i] = 16; // color 
		}
		for (i = 38; i < 42; i++){
			display_draft[20][i] = ' '; //print 
			display_att_draft[20][i] = 16; // color 
		}
		/*Plane body - static drawing - end*/
		
		
		
    display_Msg(0,0,"Distance:",maxdistance-distance);
	if(height<0)
	display_Msg(1,0,"Altitude",0);
	else
	display_Msg(1,0,"Altitude",height);
    display_Msg(2,0,"Speed:",speed);
	display_Msg(3,0,"V_angle:",v_angle);
	display_Msg(4,0,"H_angle:",h_angle);
	display_Msg(5,0,"Till Runway:",RUNWAY_START-distance);
    if((speed <=50 ) && (distance < maxdistance) && ((height <= v_angle/10)&&(height >= -v_angle/10 )) && 
	((center.x > PLANE_CENTER - 7) && (PLANE_CENTER + 7 > center.x))) //win
	{
	 if(v_angle<=30 && distance > RUNWAY_START ){
	  gameStatus = 1;
	  gameover("Nice landing,you win!!!",GREEN);
	  }
	  else{
	  gameStatus = 5;
	  gameover("You Crashed!!",RED);
	  }
	}
   else if((height < 0) && gameStatus==0){ //accident crash
       gameStatus = 5;
       gameover("You Crashed!!",RED);
    }	//crash
	
   else if(speed == 0 && height == 0 && ((center.x<PLANE_CENTER-7) || (center.x>PLANE_CENTER+7)))	//missed center
    {
     gameStatus = 4;
     gameover("You loose,you landed to far from center!",RED);
    }
	
   else if(distance > maxdistance)	//fly fast
    {
      gameStatus = 3;
      gameover("You loose,You Missed the field!",RED);
    }
	
   else if(speed <= 0 && height > 0) //stop mid air
    {
      gameStatus = 2;
      gameover("Can't stop at middle air!!!",RED);
    }
//	  display_draft[0][place] = 'g';
//	  display_att_draft[0][place] = 15;
//	  place++;
////////////////////////////////////////DISPLAY SECTION///////////////////////////////////////////////	  
	 for(i=0; i < 25; i++)
      for(j=0; j < 80; j++)
        display[i*80+j] = display_draft[i][j];
    display[2000] = '\0';
	    for(i=0; i < 25; i++)
			for(j=0; j < 80; j++)
				attr_disp[i*80+j] = display_att_draft[i][j];
    attr_disp[2000] = '\0';  
//////////////////////////////////////////////////////////////////////////////////////////////////////
  } // while(1)
} while(gameStatus==0);//do

} // updater 

int sched_arr_pid[5] = {-1};
int sched_arr_int[5] = {-1};


SYSCALL schedule(int no_of_pids, int cycle_length, int pid1, ...)
{
  int i;
  int ps;
  int *iptr;

  disable(ps);

  gcycle_length = cycle_length;
  point_in_cycle = 0;
  gno_of_pids = no_of_pids;

  iptr = &pid1;
  for(i=0; i < no_of_pids; i++)
  {
    sched_arr_pid[i] = *iptr;
    iptr++;
    sched_arr_int[i] = *iptr;
    iptr++;
  } // for
  restore(ps);

} // schedule 

xmain()
{
        int uppid, dispid, recvpid;
		asm {
		            MOV AX,3
		         	INT 10h
    	 }
	 center.x = 40;
     center.y = 5;
	 runWayEdge = center.y;
        resume( dispid = create(displayer, INITSTK, INITPRIO, "DISPLAYER", 0) );
        resume( recvpid = create(receiver, INITSTK, INITPRIO+3, "RECIVEVER", 0) );
        resume( uppid = create(updateter, INITSTK, INITPRIO, "UPDATER", 0) );
        receiver_pid =recvpid;  
        update_pid =uppid;
        disp_pid = dispid;
        set_new_int9_newisr();
    schedule(2,8, dispid, 0,  uppid, 4);
} // xmain
