import processing.serial.*;  // add the serial library
Serial myPort;               // define a serial port object to monitor
 
// define initial coordinates of cursor location
int x = 0;
int time = 0;

// Library to create CSV
Table table;

void setup() {
  size(1200, 612);           // set the window size
  println(Serial.list());    // list all available serial ports
  myPort = new Serial(this, Serial.list()[1], 1200); // type port in box
  myPort.clear();            // clear the port of any initial junk
  
  table = new Table();       // Create a table to save data as a csv file
  table.addColumn("t (s)");
  table.addColumn("T0 (C)");
  table.addColumn("T1 (C)");
}

void draw (){
 // increment lines
 stroke(0);
 // lines: x1 y1 x2 y2
 for (int i=0; i<21;){
   line(0, 544-26*i, 1200, 544-26*i);
   i++;
 }
 
 //line labels
 fill(0);
 textSize(13);
 //(text, Xcoord, Ycord) 
 for (int i=0; i<21;){
   text(nf(-20+i*9,2,1) + " C", 5, 539-26*i);
   i++;
 }
 
 //legend
 textSize(14);
 text("Target0:", 62, 585);
 text("Target1:", 62, 602);
 text("Actual0:", 167, 585);
 text("Actual1:", 167, 602);
 text("Cycles:", 310, 602);
 //circle: (x,y,diameter)
 //target0
 fill(0,0,255);
 circle(55, 580, 7);
 //target1
 fill(255,0,0);
 circle(55, 597, 7);
 //actual0
 fill(110,110,255);
 circle(160, 580, 7);
 //actual1
 fill(255,150,150);
 circle(160, 597, 7);

while (myPort.available () > 0) {  //make sure port is open
  String inString = myPort.readStringUntil('\n'); // read input string 
  if (inString != null){           //ignore null strings
    inString = trim(inString);     //trim off any whitespace
    String[] data = splitTokens(inString, ",");   // extract x & y into an array
    //proceed only if correct # of values extracted from the string:
    
    if (data.length == 5){
      int a1 = int(data[0]);  //target0
      int a2 = int(data[1]);  //target1
      int a3 = int(data[2]);  //actual0
      int a4 = int(data[3]);  //actual1
      int a5 = int(data[4]);  //cycles
      
      // CSV row labels
      TableRow newRow = table.addRow();
      // time currently hard coded to match arduino. Could be carried in automatically.
      newRow.setInt("t (s)", time);
      newRow.setInt("T0 (C)", a3);
      newRow.setInt("T1 (C)", a4);
    
      //post temps in legend
      //clear old value
      fill(210,210,210);
      stroke(210,210,210);
      rect(116,570,35,15);  //target0
      rect(116,587,35,15);  //target1
      rect(218,570,35,15);  //actual0
      rect(218,587,35,15);  //actual1
      rect(358,587,35,15);  //cycle
      //fill new value
      fill(0);
      stroke(0);
      textSize(14);
      text(a1, 120, 585);  //target0
      text(a2, 120, 602);  //target1
      text(a3, 223, 585);  //actual0
      text(a4, 223, 602);  //actual1
      text(a5, 360, 602);  //cycle
      
      //degrees to pixels conversion
      //612 boundary. 24 top, 544 bottom.
      //520 span. Curently a 180 degree span. ~2.88 pixels/degree
      a1 = int(60+(a1*2.89));
      a2 = int(60+(a2*2.89));
      a3 = int(60+(a3*2.89));
      a4 = int(60+(a4*2.89));
      //a5 (cycle) never gets plot. Only a simple counter
      
      //plot temp points
      //target0
      fill(0,0,255);
      ellipse(x, 550-a1, 8, 8);
      //target1
      fill(255,0,0);
      ellipse(x, 550-a2, 8, 8);
      //actual0
      fill(110,110,255);
      ellipse(x, 550-a3, 8, 8);
      //actual1
      fill(255,200,200); 
      ellipse(x, 550-a4, 8, 8);

      x = x + 2;
      time += 5;  //hardcoded time to match arduino sample rate
 
      //Set the bounds inside with the cursor can translate
      //This prevents the cursor from moving off the screen
      if(x > 1200){
        background(225, 225, 225); //(r,g,b)
        x = 0;
      }
    }
  }
}

 // Save data to data.csv file on local computer
 saveTable(table, "CHANGE_NAME_TO_SAVE_TemperatureLog.csv");
}
