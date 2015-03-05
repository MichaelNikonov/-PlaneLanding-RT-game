// game.java

import java.util.Scanner;

class position
{
  public int x;
  public int y;
  
} // position

class GlobalRec
{
	int PLANE_CENTER = 40;
	int LEFT_BOUND= 0;
	int RIGHT_BOUND= 80;
	int RUNWAY_START= 800;
	
	
	int point_in_cycle; //schedule variables
	int gcycle_length;//-----//---------
	int gno_of_pids;//---------//---------

	int receiver_pid,update_pid,disp_pid;
	char []display;//display serial ASCII array
	char []ch_arr;//Keyboard char buffer
	int front;//keyboard buffer pointers
	int rear;//-------//-----//--------

	//initial plane characteristics
	int speed ;
	int maxdistance;//distance to the end of runway
	int height;//height above the ground
	int distance;//flayed distance
	int v_angle;
	int h_angle;
	int gameStatus;
	int course;
	long tick ;
	String displayString;


	position center;//position of the centre of the screen - relative point of drawing objects
	int Up_L;// runway upper left point
	int Up_R ;//runway upper right point 
	int Dw_L;//runway lower left point
	int Dw_R;//runway lower right point
	int diff;//special variable that contains Dw_R - Up_L = > provides the length of the runway
	int runWayEdge;
	
   GlobalRec()
   {
    ch_arr = new char [2001];
    
	front = -1;//keyboard buffer pointers
	rear = -1;//-------//-----//--------
    
	
	//initial plane characteristics
	speed = 50;
	maxdistance= 3500;//distance to the end of runway
	height= 150;//height above the ground
	distance= 0;//flayed distance
	v_angle = 0;
	h_angle = 0;
	gameStatus = 0;
	course = 0;
	tick = 0 ; 
	center = new position();
	center.x = 40;
	center.y = 6;
	
	Up_L = -4;// runway upper left point
	Up_R =  4;//runway upper right point 
	Dw_L = -6;//runway lower left point
	Dw_R = 6;//runway lower right point
	diff = Dw_R - Up_L;
	
	runWayEdge = center.y;
     
    displayString = new String("");
   }//

} // GlobalRec


class Displayer extends Thread
{
  GlobalRec grec;

  Displayer(GlobalRec grec)
  {
   this.grec = grec;
  } // Displayer

  public void run()
  {
    synchronized(grec)
    {
     while(true)
     {
     try {
      grec.wait();
     } catch(InterruptedException ie)
       {
        System.out.println("Exception: " + ie.getMessage());
       } // catch
      System.out.println();
      System.out.print(grec.displayString);
     } // while

    } // synchronized(grec)   

  } // run

} // Displayer

class Receiver extends Thread
{
  GlobalRec grec;

  Receiver(GlobalRec grec)
  {
   this.grec = grec;
  } // Updater

  public void run()
  {
    int i;
    String str;
    Scanner input = new Scanner(System.in);
    char ch;

    while(true)
    {
       str = input.nextLine();
       str = str.trim();

      synchronized(grec)
      {

       for(i =0; i < str.length(); i++)
       {
          ch = str.charAt(i);
 
          grec.rear++;
          grec.ch_arr[grec.rear] = ch;
          if (grec.front == -1)
              grec.front = 0;
       } // for   
      } // synchronized(grec)

    } // while

  } // run
  
} // Receiver


class Updater extends Thread
{
  GlobalRec grec;
  int dist;
  char [][] displayDraft;//display two dimension ASCII array
   final int rows = 25;
   final int cols = 80;
    

  Updater(GlobalRec grec)
  {
	  this.displayDraft = new char[rows][cols];
	  this.grec = grec;
  } // Updater

  public void run()
  {
	
   

    while(true)
    {
      int i, j;
      char ch;
      
      synchronized(grec)
      {
     //  try {
     //   grec.wait();
     //  } catch(InterruptedException ie)
     //  {
     //   System.out.println("Exception: " + ie.getMessage());
     //  } // catch

     
        while(true)
        {
          grec.height = grec.height -  grec.v_angle/10 ;
      	  grec.center.x = grec.center.x - grec.h_angle/10;
      	  grec.distance = grec.distance + grec.speed ;
      	  grec.Dw_L = grec.Dw_L - grec.speed/50;
      	  grec.Dw_R = grec.Dw_R + grec.speed/50;
      	  
      	  if(grec.v_angle!= 0)
      	     if(grec.tick % 4 == 1)
      		 {
      	    	 grec.Up_L--;
      	    	 grec.Up_R++;
      	    	 grec.Dw_L--;
      	    	 grec.Dw_R++;
      	    	 grec.speed--;
      		 }
      	    
      	
      	  grec.tick++;
          
          try
          {
           grec.wait();
          } 
          
          catch(InterruptedException ie)
            {
             System.out.println("Exception: " + ie.getMessage());
            } // catch
           
          while(grec.front != -1)
          {
           
       	 ch = grec.ch_arr[grec.front];
       	 if(grec.front != grec.rear)
                  grec.front++;
            else
            {
       		  grec.front = -1;
       		  grec.rear = -1;
       	   }
                 
            if (((ch == 'a') || (ch == 'A'))) 
       	       {
       		    grec.h_angle = grec.h_angle - 10;
       			//course = (course -1)%360;
       		   }
            else if (((ch == 'd') || (ch == 'D')))
       		   {
       		    grec.h_angle = grec.h_angle + 10;
       			//course = (course +1)%360;
       		   }
            else if ( (ch == 'w') || (ch == 'W') )
       	      {
       		   grec.v_angle = grec.v_angle  + 10;
       		   }
       /////
            else if((ch=='s' || ch=='S')){
       	    grec.v_angle = grec.v_angle - 10;
       	    
       		if(grec.v_angle < 0)
       		   grec.v_angle = 0;
       	 }     
       	 
       	// else if ( (ch == 't') || (ch == 'T') )
       	//    speed = speed + 50;
       	 else if ( (ch == 'g') || (ch == 'G'))
       	    grec.speed = grec.speed - 20;	 
       ////////////////////////////////////////////*/ 
             
        }// while(front != -1)

          for(i=0; i < rows; i++)
             for(j=0; j < cols; j++)
                displayDraft[i][j] = ' ';
                    
                        
   		
           // The sky
             grec.diff = grec.Dw_R - grec.Up_R;
             if(grec.diff>19)
            	 grec.diff=19 ;
         	for (i = 0; i < grec.center.y; i++)
         			for (j=0; j< 80; j++ )
         				displayDraft[i][j] = '-'; //print 
         			
         			
         	//The ground
         	for (i = grec.center.y; i < 25; i++)
         		for (j=0; j< 80; j++ )
         			displayDraft[i][j] = '*'; //print 
         		
            //The road init	
         	dist = -1;
         	
         	for (i = grec.center.y; i < (grec.center.y + grec.diff); i++)
         	{
         		dist++;
         		
         		for (j = (grec.center.x + grec.Up_L - dist ); j <= (grec.center.x + grec.Up_R + dist)&& j< 80; j++)
         			{
         			if(j  >= 0 )
         			{
         				if(j == grec.center.x)
         					displayDraft[i][j] ='|';
         					
         				else if(j==(grec.center.x+ grec.Up_L - dist))
         					displayDraft[i][j] ='/';
         				
         				else if(j==(grec.center.x+ grec.Up_R + dist))
         					displayDraft[i][j] ='\\';
         								
         				else
         					displayDraft[i][j] = ' '; //print 
         			  }	
         			}
         	}
         	
            /*Plane body - static drawing - start - light blue BG & font*/
         		for (i = 1; i < 79; i++)
         			displayDraft[24][i] = '%'; //print 

         		for (i = 5; i < 75; i++)
         			displayDraft[23][i] = '%'; //print 

         		for (i = 15; i < 65; i++)
         			displayDraft[22][i] = '%'; //print 
         	
         		for (i = 30; i < 50; i++)
         			displayDraft[21][i] = '%'; //print 
         		
         		for (i = 38; i < 42; i++)
         			displayDraft[20][i] = '%'; //print 
         		
         		
         	this.displayMsg(0, 0,"Speed : " + grec.speed);	
         	this.displayMsg(1, 0,"Distance : " + (grec.maxdistance - grec.distance) );
         	this.displayMsg(2, 0,"Altitude : " + grec.height);
         	this.displayMsg(3, 0,"V_angle : " + grec.v_angle);
         	this.displayMsg(4, 0,"H_angle : " + grec.h_angle);	
         	this.displayMsg(5, 0,"Till Runway : " + grec.maxdistance);	
         		
         		
         	   grec.displayString = " ";
               for(i=0; i < rows; i++)
                  for(j=0; j < cols; j++){
                	   grec.displayString += displayDraft[i][j];
                	   if(j == 79)
                		   grec.displayString += '\n';
                	    }
                  // grec.displayString += '\0';  

         		/*Plane body - static drawing - end*/
         		
         } // while

      } // synchronized(grec)
    } // while

  } // run
  public  void displayMsg(int row, int col, String msg)
   { 
	   for (int i =0; i< msg.length(); i++)
		   	this.displayDraft[row][col + i] = msg.charAt(i);       
   }
       
}// Updater



class Scheduler extends Thread
{
  GlobalRec grec;

  Scheduler(GlobalRec grec)
  {
   this.grec = grec;
  } // Updater
  
 public void run()
 {

 while(true)
 {
   synchronized(grec)
    {
     grec.notify();
    }// synchronized(grec)

    try
    {   
     Thread.currentThread().sleep(500);
    } // try
    catch (InterruptedException e)
    {
     System.out.println("Exception: " + e.getMessage());
    } // catch 
  } // while
 } // run 

} // Scheduler

public class game 
{
    public static void main(String[] args) throws InterruptedException 
    {
     GlobalRec grec = new GlobalRec();

     Scheduler sch = new Scheduler(grec);
     Updater upd = new Updater(grec);
     Receiver rcv = new Receiver(grec);
     Displayer disp = new Displayer(grec);
     rcv.setPriority(Thread.MAX_PRIORITY);
     sch.setPriority(Thread.MAX_PRIORITY);

     upd.start();     
     disp.start();
     rcv.start();
     sch.start();

    
     
    }// main
} // game
