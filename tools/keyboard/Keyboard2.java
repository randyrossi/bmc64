import java.awt.*;
import java.io.*;
import java.awt.event.*;
import java.util.List;
import java.util.ArrayList;

public class Keyboard2 //extends Frame
{
  static int scale = 2;
  static int W = 1024/scale;
  static int H = 328/scale;

  int keys_widths[] = {54,81,108,472,82};
  int xc[] = {14,940, 14, 98,828,940, 0,728,940, 0, 56,140,700,940,192,700};
  int yc[] = { 0,  0, 70, 70, 70, 70,140,140,140,210,210,210,210,210,280,280};
  int nc[] = {16,  1,  1, 13,  1,  1, 13,  1,  1,  1,  1, 10,  1,  1,  1, 4};
  int wc[] = { 0,  4,  1,  0,  1,  4,  0,  2,  4,  0,  1,  0,  1,  4,  3, 0};
  int rc[]=  { 0,  0,  1,  1,  1,  1,  2,  2,  2,  3,  3,  3,  3,  3,  4, 4};

  static int UNUSED = -1;
  static int HOME = -2;
  static int DEL = -3;
  static int F1 = -4;
  static int F2 = -5;
  static int F3 = -6;
  static int F4 = -7;
  static int F5 = -8;
  static int F6 = -9;
  static int F7 = -10;
  static int F8 = -11;
  static int CNTRL = -12;
  static int RESTORE = -13;
  static int RUNSTOP = -14;
  static int SHIFTLOCK = -15;
  static int RETURN = -16;
  static int COMMODORE = -17;
  static int LSHIFT = -18;
  static int RSHIFT = -19;
  static int CURSDOWN = -20;
  static int CURSRIGHT = -21;
  static int SPACE = -22;
  static int CLR = -23;
  static int INS = -24;
  static int ESC = -25;
  static int CURSUP = -26;
  static int CURSLEFT = -27;

  int code[][] = {
    {ESC,49,50,51,52,53,54,55,56,57,48,43,45,61,HOME,DEL},
    {F1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {CNTRL,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {17,23,5,18,20,25,21,9,15,16,0,28,42,-1,-1,-1},
    {CNTRL,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {RUNSTOP,SHIFTLOCK,1,19,4,6,7,8,10,11,12,58,59,-1,-1,-1},
    {RETURN,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {COMMODORE,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {LSHIFT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {26,24,3,22,2,14,13,44,46,47,-1,-1,-1,-1,-1,-1},
    {RSHIFT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {SPACE,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {CURSUP,CURSDOWN,CURSLEFT,CURSRIGHT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  };

  int shift_code[][] = {
    {ESC,33,34,35,36,37,38,39,40,41,48,91,93,31,CLR,INS},
    {F2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {CNTRL,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {17+0x40,23+0x40,5+0x40,18+0x40,20+0x40,25+0x40,21+0x40,9+0x40,15+0x40,16+0x40,122,105,64,-1,-1,-1},
    {CNTRL,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {RUNSTOP,SHIFTLOCK,1+0x40,19+0x40,4+0x40,6+0x40,7+0x40,8+0x40,10+0x40,11+0x40,12+0x40,27,29,-1,-1,-1},
    {RETURN,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {COMMODORE,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {LSHIFT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {26+0x40,24+0x40,3+0x40,22+0x40,2+0x40,14+0x40,13+0x40,60,62,63,-1,-1,-1,-1,-1,-1},
    {RSHIFT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {SPACE,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {CURSUP,CURSDOWN,CURSLEFT,CURSRIGHT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  };

  int comm_code[][] = {
    {ESC,49,50,51,52,53,54,55,56,57,48,43,45,94,CLR,INS},
    {F2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {CNTRL,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {107,115,113,114,99,119,120,98,121,111,100,104,95,-1,-1,-1},
    {CNTRL,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {RUNSTOP,SHIFTLOCK,112,110,108,123,101,116,117,97,118,27,29,-1,-1,-1},
    {RETURN,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {COMMODORE,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {LSHIFT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {109,125,124,126,127,106,103,60,62,63,-1,-1,-1,-1,-1,-1},
    {RSHIFT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {F8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {SPACE,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
    {CURSUP,CURSDOWN,CURSLEFT,CURSRIGHT,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
  };

  String plus4_row_col_vals[][] = {
    {"77","70","73","10","13","20","23","30","33","40","43","66","56","65","71","00"},
    {"04"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"72"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"76","11","16","21","26","31","36","41","46","51","07","02","61"," "," "," "},
    {"CONTROL"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"06"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"77","SHIFTLOCK","12","15","22","25","32","35","42","45","52","55","62","65"," "," "},
    {"01"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"05"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"75"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"17"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"14","27","24","37","34","47","44","57","54","67"," "," "," "," "," "," "},
    {"17"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"03"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"74"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"53","50","60","63"," "," "," "," "," "," "," "," "," "," "," "," "},
  };

  String label[][] = {
    {"Esc","1","2","3","4","5","6","7","8","9","0","Plus","Minus","Equals","Home","Del"},
    {"F1"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"Cntrl"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"Q","W","E","R","T","Y","U","I","O","P","At","Pound","Asterisk"," "," "," "},
    {"Cntrl"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"F3"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"RunStop","ShiftLock","A","S","D","F","G","H","J","K","L","Colon","SemiColon"," "," "," "},
    {"Return"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"F5"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"Commodore"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"LeftShift"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"Z","X","C","V","B","N","M","Comma","Period","Slash"," "," "," "," "," "," "},
    {"RightShift"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"F7"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"Space"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "},
    {"CursorUp","CursorDown","CursorLeft","CursorRight"," "," "," "," "," "," "," "," "," "," "," "," "},
  };

  public static void main(String args[]) {
    final Keyboard2 d = new Keyboard2();
    d.gen(args[0]);
  }

  public void gen(String prefix) {

    System.out.println("/* Generated by Keyboard.java */");

    System.out.println("#ifndef VIRTUAL_KEYBOARD_H");
    System.out.println("#define VIRTUAL_KEYBOARD_H");
    System.out.println("");
    System.out.println("struct vkbd_key {");
    System.out.println("  int x;");
    System.out.println("  int y;");
    System.out.println("  int w;");
    System.out.println("  int h;");
    System.out.println("  int row;");
    System.out.println("  int col;");
    System.out.println("  int layout_row;");
    System.out.println("  int layout_col;");
    System.out.println("  int toggle;");
    System.out.println("  int code;");
    System.out.println("  int shift_code;");
    System.out.println("  int comm_code;");
    System.out.println("  int up;");
    System.out.println("  int down;");
    System.out.println("  int left;");
    System.out.println("  int right;");
    System.out.println("  int state;");
    System.out.println("};");
    System.out.println("");

    System.out.println("struct vkbd_key vkbd[] = {");
    int total = 0;
    int total_in_row = 0;
    int layout_col = -1;
    int layout_row = -1;

    int minx = 9999;
    int maxx =0;
    int miny = 9999;
    int maxy =0;

    int totals_in_row[] = new int[5];

    List<Key> keys = new ArrayList<Key>();

    String row_col_vals[][] = null;
    if (prefix.equals("plus4")) {
       row_col_vals = plus4_row_col_vals;
    } else {
       System.out.println("Unknown model " + prefix);
       System.exit(-1);
    }

    for (int i=0;i<xc.length;i++) {
       if (layout_row != rc[i]) {
          layout_col = 0;
          if (layout_row >=0) { totals_in_row[layout_row] = total_in_row; }
          total_in_row = 0;
       }
       layout_row = rc[i];
       for (int j=0;j<nc[i];j++) {
         int x = xc[i]/scale + j*56/scale;
         int y = yc[i]/scale;
         int w = keys_widths[wc[i]]/scale;
         int h = 67/scale;

         if (x < minx) minx = x;
         if (x+w > maxx) maxx = x+w;
         if (y < miny) miny = y;
         if (y+h > maxy) maxy = y+h;
         
         int row,col;
         int toggle = 0;
         if (row_col_vals[i][j].equals("RESTORE")) {
           col = -3;
           row = -0;
         } else if (row_col_vals[i][j].equals("SHIFTLOCK")) {
           col = -4;
           row = -1;
           toggle = 1;
         } else {
           col = row_col_vals[i][j].charAt(0) - '0';
           row = row_col_vals[i][j].charAt(1) - '0';
           if (label[i][j].equals("Commodore")) toggle=1;
           else if (label[i][j].equals("Cntrl")) toggle=1;
           else if (label[i][j].equals("LeftShift")) toggle=1;
           else if (label[i][j].equals("RightShift")) toggle=1;
           else if (label[i][j].equals("RunStop")) toggle=1;
         }

         Key k = new Key();
         k.x = x;
         k.y = y;
         k.w = w;
         k.h = h;
         k.row = row;
         k.col = col;
         k.layout_row = layout_row;
         k.layout_col = layout_col;
         k.label = label[i][j];
         k.index = total;
         k.toggle = toggle;
         k.code = code[i][j];
         k.shift_code = shift_code[i][j];
         k.comm_code = comm_code[i][j];
         keys.add(k);

         layout_col++; total++; total_in_row++;
       }
    }
    if (layout_row >=0) { totals_in_row[layout_row] = total_in_row; }

    // Fill in nav for keys
    for (int i=0;i<keys.size();i++) {
       Key k = keys.get(i);
       int cx = k.x + k.w/2;
       int cy = k.y + k.h/2;

       Key k2 = null;
       double mindist = Double.MAX_VALUE;
       int mink = i;
       for (int j=0;j<keys.size();j++) {
         if (j==i) continue;
         k2 = keys.get(j);
         int cx2 = k2.x + k2.w/2;
         int cy2 = k2.y + k2.h/2;
         double dist = Math.sqrt((cx-cx2)*(cx-cx2) + (cy-cy2)*(cy-cy2));
         if (dist < mindist && cy2 < cy) { mindist = dist; mink = j; }
       }
       k.up = mink;

       k2 = null;
       mindist = Double.MAX_VALUE;
       mink = i;
       for (int j=0;j<keys.size();j++) {
         if (j==i) continue;
         k2 = keys.get(j);
         int cx2 = k2.x + k2.w/2;
         int cy2 = k2.y + k2.h/2;
         double dist = Math.sqrt((cx-cx2)*(cx-cx2) + (cy-cy2)*(cy-cy2));
         if (dist < mindist && cy2 > cy) { mindist = dist; mink = j; }
       }
       k.down = mink;

       k.left = k.index - 1;
       if (k.left < 0) k.left = 0;

       k.right = k.index + 1;
       if (k.right >= total) k.right = total-1;
    }
 

    for (int i=0;i<keys.size();i++) {
       Key k = keys.get(i);
       System.out.print("   {");
       System.out.print(k.x+" ,");
       System.out.print(k.y+" ,");
       System.out.print(k.w+" ,");
       System.out.print(k.h+" ,");
       System.out.print(k.row+" ,");
       System.out.print(k.col+" ,");
       System.out.print(k.layout_row+" ,");
       System.out.print(k.layout_col+" ,");
       System.out.print(k.toggle+" ,");
       System.out.print(k.code+" ,");
       System.out.print(k.shift_code+" ,");
       System.out.print(k.comm_code+" ,");
       System.out.print(k.up+" ,");
       System.out.print(k.down+" ,");
       System.out.print(k.left+" ,");
       System.out.print(k.right+" ,");
       System.out.print("0 ,");
       System.out.print("}, ");
       System.out.println(" /* "+i+ " " +k.label+" */");
    }

    System.out.println(" };");
    System.out.println("");

/*
    System.out.print("int max_col_for_row[] = { ");
    for (int i=0;i<totals_in_row.length;i++) {
      System.out.print(totals_in_row[i]+",");
    }
    System.out.println("};");
*/

    System.out.println("");
    System.out.println("#define NUM_KEYS "+total);
    System.out.println("#define VKBD_WIDTH "+(maxx-minx+1));
    System.out.println("#define VKBD_HEIGHT "+(maxy-miny+1));
    System.out.println("");
    System.out.println("// special code values for keys");
    System.out.println("#define VKBD_KEY_HOME "+HOME);
    System.out.println("#define VKBD_DEL "+DEL);
    System.out.println("#define VKBD_F1 "+F1);
    System.out.println("#define VKBD_F3 "+F3);
    System.out.println("#define VKBD_F5 "+F5);
    System.out.println("#define VKBD_F7 "+F7);
    System.out.println("#define VKBD_CNTRL "+CNTRL);
    System.out.println("#define VKBD_RESTORE "+RESTORE);
    System.out.println("#define VKBD_RUNSTOP "+RUNSTOP);
    System.out.println("#define VKBD_SHIFTLOCK "+SHIFTLOCK);
    System.out.println("#define VKBD_RETURN "+RETURN);
    System.out.println("#define VKBD_COMMODORE "+COMMODORE);
    System.out.println("#define VKBD_LSHIFT "+LSHIFT);
    System.out.println("#define VKBD_RSHIFT "+RSHIFT);
    System.out.println("#define VKBD_CURSDOWN "+CURSDOWN);
    System.out.println("#define VKBD_CURSRIGHT "+CURSRIGHT);
    System.out.println("#define VKBD_SPACE "+SPACE);
    System.out.println("#define VKBD_CLR "+CLR);
    System.out.println("#define VKBD_INS "+INS);
    System.out.println("#define VKBD_ESC "+ESC);
    System.out.println("#define VKBD_CURSUP "+CURSUP);
    System.out.println("#define VKBD_CURSLEFT "+CURSLEFT);
    System.out.println("#endif  // VIRTUAL_KEYBOARD_H");
  }
}
