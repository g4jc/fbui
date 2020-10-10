
/*=========================================================================
 *
 * fbclock, an analog clock for FBUI (in-kernel framebuffer UI)
 * Copyright (C) 2004 Zachary Smith, fbui@comcast.net
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this module; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * (See the file COPYING in the main directory of this archive for
 * more details.)
 *
 *=======================================================================*/


/* Changes
 *
 * 26 Sep, 2004: responds to geometry changes.
 * 30 Sep, 2004: lib changes.
 * 07 Sep, 2005: added icon
 */


#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include "libfbui.h"


static short w = 200;
static short h = 200;

int getpos (int deg, int *xp, int *yp, int border)
{
	double radians;
	double x,y,xradius,yradius;

	radians = deg;
	radians /= 360.0;
	radians *= 2.0 * 3.14159265;

	xradius = w / 2 - border;
	yradius = h / 2 - border;
	x = sin (radians) * xradius;
	y = cos (radians) * yradius;
	x += w/2;
	y = h/2 - y;

	*xp = x;
	*yp = y;
}


/* XPM */
static char * clock_xpm[] = {
"24 32 402 2",
"  	c #3CB039",
". 	c #3DB039",
"+ 	c #3CB03A",
"@ 	c #3DB139",
"# 	c #41B138",
"$ 	c #43B337",
"% 	c #46B337",
"& 	c #49B536",
"* 	c #4DB635",
"= 	c #50B735",
"- 	c #53B833",
"; 	c #56BA33",
"> 	c #5ABA32",
", 	c #5DBC31",
"' 	c #60BD30",
") 	c #63BE30",
"! 	c #66BF2E",
"~ 	c #6AC12E",
"{ 	c #6DC22D",
"] 	c #70C32C",
"^ 	c #73C42B",
"/ 	c #76C52B",
"( 	c #3FB139",
"_ 	c #41B238",
": 	c #45B337",
"< 	c #47B536",
"[ 	c #4BB636",
"} 	c #4EB735",
"| 	c #51B834",
"1 	c #55B933",
"2 	c #58BA32",
"3 	c #5BBC31",
"4 	c #5EBC31",
"5 	c #61BE30",
"6 	c #65BF2F",
"7 	c #68C02F",
"8 	c #6BC12E",
"9 	c #6EC22D",
"0 	c #71C42C",
"a 	c #74C52B",
"b 	c #77C62A",
"c 	c #7BC729",
"d 	c #3DB03A",
"e 	c #3CB13A",
"f 	c #43B338",
"g 	c #46B436",
"h 	c #4CB636",
"i 	c #4FB735",
"j 	c #59BB32",
"k 	c #5CBB32",
"l 	c #5FBD31",
"m 	c #66C02F",
"n 	c #69C02E",
"o 	c #6CC22D",
"p 	c #6FC32C",
"q 	c #72C42B",
"r 	c #76C62B",
"s 	c #79C62A",
"t 	c #7CC729",
"u 	c #7FC928",
"v 	c #3EB139",
"w 	c #44B238",
"x 	c #48B437",
"y 	c #4AB536",
"z 	c #4EB635",
"A 	c #51B734",
"B 	c #54B933",
"C 	c #57BA33",
"D 	c #5BBB31",
"E 	c #61BD31",
"F 	c #64BF2F",
"G 	c #67C02E",
"H 	c #6AC12D",
"I 	c #70C42C",
"J 	c #73C52B",
"K 	c #7AC72A",
"L 	c #7DC829",
"M 	c #81C928",
"N 	c #84CA27",
"O 	c #42B338",
"P 	c #45B436",
"Q 	c #4CB635",
"R 	c #4FB634",
"S 	c #52B834",
"T 	c #59BA32",
"U 	c #5CBC31",
"V 	c #5EBD30",
"W 	c #62BE2F",
"X 	c #65C02F",
"Y 	c #68C12F",
"Z 	c #6BC22D",
"` 	c #6FC22D",
" .	c #72C42C",
"..	c #75C52B",
"+.	c #78C62A",
"@.	c #82CA28",
"#.	c #85CB27",
"$.	c #88CC26",
"%.	c #44B337",
"&.	c #46B437",
"*.	c #49B535",
"=.	c #50B834",
"-.	c #53B933",
";.	c #56BA32",
">.	c #62BE3B",
",.	c #7AC852",
"'.	c #94D26F",
").	c #A6DA81",
"!.	c #ADDE88",
"~.	c #AFDE88",
"{.	c #ABDC80",
"].	c #A0D86C",
"^.	c #8CCE4E",
"/.	c #7CC834",
"(.	c #79C72A",
"_.	c #7CC829",
":.	c #83CA27",
"<.	c #86CB26",
"[.	c #89CC26",
"}.	c #8CCD25",
"|.	c #4CB535",
"1.	c #4FB734",
"2.	c #58BA33",
"3.	c #75C64F",
"4.	c #B0DE92",
"5.	c #E6F5D1",
"6.	c #FBFEEA",
"7.	c #FFFFEF",
"8.	c #FCFEEA",
"9.	c #E9F6D0",
"0.	c #BEE38F",
"a.	c #92D148",
"b.	c #81CA28",
"c.	c #88CB27",
"d.	c #8BCD25",
"e.	c #8ECE25",
"f.	c #91CF24",
"g.	c #53B934",
"h.	c #56B933",
"i.	c #5CBD35",
"j.	c #83CB5E",
"k.	c #DDF1C6",
"l.	c #FEFFEE",
"m.	c #E4F4C4",
"n.	c #A3D756",
"o.	c #8CCC2C",
"p.	c #8CCE25",
"q.	c #8FCF24",
"r.	c #93D023",
"s.	c #96D123",
"t.	c #54B833",
"u.	c #57BA32",
"v.	c #8ED26B",
"w.	c #EEF8DB",
"x.	c #F2FAD9",
"y.	c #AFDD62",
"z.	c #90CF24",
"A.	c #97D222",
"B.	c #9AD322",
"C.	c #78C74D",
"D.	c #E7F5D2",
"E.	c #EEF8D0",
"F.	c #A5D843",
"G.	c #98D222",
"H.	c #9CD321",
"I.	c #9FD420",
"J.	c #5ABB31",
"K.	c #66C03A",
"L.	c #D2EDB8",
"M.	c #E2F2B4",
"N.	c #A2D629",
"O.	c #9FD520",
"P.	c #A3D61F",
"Q.	c #5EBD31",
"R.	c #62BE30",
"S.	c #92D268",
"T.	c #F6FBE3",
"U.	c #6CA966",
"V.	c #B4D3A9",
"W.	c #F9FCE2",
"X.	c #BCE35D",
"Y.	c #A4D61F",
"Z.	c #A8D71E",
"`.	c #66BF2F",
" +	c #C7E8A7",
".+	c #FDFEED",
"++	c #8EBC84",
"@+	c #FEFFED",
"#+	c #DEF0A1",
"$+	c #A8D81E",
"%+	c #ACD91D",
"&+	c #E1F3C7",
"*+	c #EDF7C4",
"=+	c #ADDA1D",
"-+	c #B0DA1C",
";+	c #EBF7D3",
">+	c #A9CC9E",
",+	c #F3FAD1",
"'+	c #B1DB1B",
")+	c #B5DC1A",
"!+	c #74C52C",
"~+	c #EDF7D6",
"{+	c #71AC69",
"]+	c #257F22",
"^+	c #F5FAD4",
"/+	c #B6DD1B",
"(+	c #BADE19",
"_+	c #75C42B",
":+	c #EAF6CF",
"<+	c #83B77C",
"[+	c #589C52",
"}+	c #A0C795",
"|+	c #F4FACD",
"1+	c #BBDF1A",
"2+	c #BEE018",
"3+	c #7AC729",
"4+	c #7DC729",
"5+	c #DFF1BC",
"6+	c #FFFFEE",
"7+	c #63A35C",
"8+	c #C2DBB5",
"9+	c #EEF7B8",
"0+	c #C0E118",
"a+	c #C2E217",
"b+	c #7EC929",
"c+	c #C4E690",
"d+	c #FCFEEB",
"e+	c #EDF4DE",
"f+	c #51994C",
"g+	c #FEFEEA",
"h+	c #E1F189",
"i+	c #C3E217",
"j+	c #C7E317",
"k+	c #82CA27",
"l+	c #86CB27",
"m+	c #9CD547",
"n+	c #F2F9D8",
"o+	c #A0C695",
"p+	c #7CB174",
"q+	c #F8FCD7",
"r+	c #CDE73B",
"s+	c #C8E316",
"t+	c #CBE515",
"u+	c #86CC26",
"v+	c #8ACD25",
"w+	c #8DCE25",
"x+	c #CCE992",
"y+	c #FEFEE9",
"z+	c #E5F28B",
"A+	c #CAE415",
"B+	c #CCE514",
"C+	c #D0E614",
"D+	c #8ECE24",
"E+	c #92D023",
"F+	c #95D123",
"G+	c #DFF1B0",
"H+	c #FFFFED",
"I+	c #EEF6AD",
"J+	c #CAE515",
"K+	c #CEE615",
"L+	c #D1E714",
"M+	c #D4E813",
"N+	c #93D024",
"O+	c #99D222",
"P+	c #A9D938",
"Q+	c #E6F4B7",
"R+	c #FEFFEC",
"S+	c #F0F8B4",
"T+	c #D2E82E",
"U+	c #CFE614",
"V+	c #D2E713",
"W+	c #D5E913",
"X+	c #D9EA12",
"Y+	c #94D023",
"Z+	c #97D122",
"`+	c #9AD321",
" @	c #9ED420",
".@	c #A1D520",
"+@	c #D7ED8F",
"@@	c #FBFDE5",
"#@	c #FDFEE5",
"$@	c #E6F38B",
"%@	c #CDE514",
"&@	c #D3E813",
"*@	c #D7E912",
"=@	c #DAEA11",
"-@	c #DDEB10",
";@	c #A2D620",
">@	c #A5D71F",
",@	c #ABD91D",
"'@	c #BCE140",
")@	c #E1F19B",
"!@	c #F7FBD7",
"~@	c #FDFEE9",
"{@	c #F8FCD6",
"]@	c #EAF499",
"^@	c #D4E839",
"/@	c #D2E714",
"(@	c #D5E812",
"_@	c #D8EA12",
":@	c #DBEA11",
"<@	c #DFEC10",
"[@	c #E1ED10",
"}@	c #9DD421",
"|@	c #A6D81E",
"1@	c #AAD81E",
"2@	c #ADD91C",
"3@	c #B0DB1C",
"4@	c #B3DC1B",
"5@	c #B6DD1A",
"6@	c #B9DF19",
"7@	c #BDE019",
"8@	c #C0E018",
"9@	c #C3E218",
"0@	c #C9E415",
"a@	c #CDE515",
"b@	c #D6E813",
"c@	c #DCEB11",
"d@	c #E0EC10",
"e@	c #E3ED0F",
"f@	c #E6EE0E",
"g@	c #A5D71E",
"h@	c #ABD91E",
"i@	c #AEDA1D",
"j@	c #B5DC1B",
"k@	c #B8DE1A",
"l@	c #BBDE1A",
"m@	c #C1E118",
"n@	c #C5E216",
"o@	c #C8E416",
"p@	c #CBE415",
"q@	c #CEE514",
"r@	c #D1E614",
"s@	c #D7EA12",
"t@	c #DEEB10",
"u@	c #E4EE0F",
"v@	c #E7F00E",
"w@	c #EBF00D",
"x@	c #A6D71E",
"y@	c #A9D81E",
"z@	c #ACDA1D",
"A@	c #AFDB1C",
"B@	c #B9DE1A",
"C@	c #BCDF19",
"D@	c #BFE018",
"E@	c #C2E117",
"F@	c #C6E216",
"G@	c #D2E813",
"H@	c #D6E912",
"I@	c #E2ED0F",
"J@	c #E5EE0F",
"K@	c #E9EF0E",
"L@	c #ECF10D",
"M@	c #EFF20C",
"N@	c #AEDA1C",
"O@	c #B1DB1C",
"P@	c #B4DC1B",
"Q@	c #B7DE1B",
"R@	c #BADF19",
"S@	c #BEDF19",
"T@	c #C1E117",
"U@	c #C7E316",
"V@	c #D0E714",
"W@	c #E4ED0F",
"X@	c #E7EF0E",
"Y@	c #EAF00D",
"Z@	c #EDF10D",
"`@	c #F0F30C",
" #	c #F4F40B",
".#	c #AFDA1C",
"+#	c #B2DC1B",
"@#	c #BEE118",
"##	c #C2E118",
"$#	c #C5E316",
"%#	c #D8E912",
"&#	c #E1ED0F",
"*#	c #E8F00D",
"=#	c #EBF10D",
"-#	c #EFF20D",
";#	c #F1F30B",
">#	c #F5F40B",
",#	c #F8F60A",
"'#	c #B3DC1C",
")#	c #BDDF19",
"!#	c #C3E117",
"~#	c #C6E316",
"{#	c #CAE416",
"]#	c #CCE515",
"^#	c #E6EF0E",
"/#	c #EAF00E",
"(#	c #ECF10C",
"_#	c #F3F30B",
":#	c #F6F50A",
"<#	c #F8F50A",
"[#	c #BEE019",
"}#	c #C4E216",
"|#	c #D5E813",
"1#	c #DBEB11",
"2#	c #E2EC0F",
"3#	c #E5EE0E",
"4#	c #EEF20D",
"5#	c #F8F509",
"  . . + + @ # $ % & * = - ; > , ' ) ! ~ { ] ^ / ",
"    +   ( _ : < [ } | 1 2 3 4 5 6 7 8 9 0 a b c ",
"d + e ( f g & h i - ; j k l ) m n o p q r s t u ",
"+ v # w x y z A B C D 4 E F G H { I J b K L M N ",
"( O P & Q R S 1 T U V W X Y Z `  ...+.t u @.#.$.",
"%.&.*.* =.-.;.>.,.'.).!.~.{.].^./.(._.u :.<.[.}.",
"x |.1.A B 2.3.4.5.6.7.7.7.7.8.9.0.a.b.#.c.d.e.f.",
"Q = g.h.i.j.k.l.7.7.7.7.7.7.7.7.l.m.n.o.p.q.r.s.",
"| t.u.3 v.w.7.7.7.7.7.7.7.7.7.7.7.7.x.y.z.r.A.B.",
"h.T U C.D.7.7.7.7.7.7.7.7.7.7.7.7.7.7.E.F.G.H.I.",
"J., K.L.l.7.7.7.7.7.7.7.7.7.7.7.7.7.7.l.M.N.O.P.",
"Q.R.S.T.7.7.7.7.7.7.7.7.7.7.7.U.V.7.7.7.W.X.Y.Z.",
"W `. +.+7.7.7.7.7.7.7.7.7.7.++++7.7.7.7.@+#+$+%+",
"G ~ &+7.7.7.7.7.7.7.7.7.7.++++7.7.7.7.7.7.*+=+-+",
"o 9 ;+7.7.7.7.7.7.7.7.7.>+U.7.7.7.7.7.7.7.,+'+)+",
"] !+~+7.7.7.7.7.7.7.7.{+]+7.7.7.7.7.7.7.7.^+/+(+",
"_++.:+7.7.7.7.7.7.7.7.<+[+}+7.7.7.7.7.7.7.|+1+2+",
"3+4+5+6+7.7.7.7.7.7.7.7.7.7+8+7.7.7.7.7.6+9+0+a+",
"b+M c+d+7.7.7.7.7.7.7.7.7.e+f+7.7.7.7.7.g+h+i+j+",
"k+l+m+n+7.7.7.7.7.7.7.7.7.7.o+p+7.7.7.7.q+r+s+t+",
"u+v+w+x+8.7.7.7.7.7.7.7.7.7.7.++++7.7.y+z+A+B+C+",
"d.D+E+F+G+@+7.7.7.7.7.7.7.7.7.7.U.V.H+I+J+K+L+M+",
"z.N+s.O+P+Q+R+7.7.7.7.7.7.7.7.7.7.R+S+T+U+V+W+X+",
"Y+Z+`+ @.@Y.+@@@7.7.7.7.7.7.7.6+#@$@%@C+&@*@=@-@",
"G.H.I.;@>@$+,@'@)@!@~@@+@+~@{@]@^@U+/@(@_@:@<@[@",
"}@.@Y.|@1@2@3@4@5@6@7@8@9@j+0@a@C+&@b@X+c@d@e@f@",
".@g@Z.h@i@'+j@k@l@2+m@n@o@p@q@r@(@s@:@t@[@u@v@w@",
"x@y@z@A@4@/+B@C@D@E@F@0@a@U+G@H@X+c@<@I@J@K@L@M@",
",@N@O@P@Q@R@S@T@i+U@A+q@V@&@*@=@-@[@W@X@Y@Z@`@ #",
".#+#j@k@C@@###$#o@t+U+/@(@%#c@<@&#J@*#=#-#;#>#,#",
"'#5@R@)#8@!#~#{#]#C+M+*@=@-@d@e@^#/#(#M@_#:#<#,#",
"k@1+[#m@}#U@t+K+V+|#%#1#t@2#3#X@w@4#;# #<#5#<#<#"};

int
main(int argc, char** argv)
{
	int i;
	Display *dpy;
	Window *win;

	unsigned long markercolor = 0x00ff00;
	unsigned long centercolor = 0xffff00;
	unsigned long handcolor = 0x00ffff;
	unsigned long gemcolor = 0xffffff;
	unsigned long bgcolor = 0x2000;

	dpy = fbui_display_open ();
	if (!dpy)
		FATAL("cannot open display");

	w = 120;
	h = 120;
	win = fbui_window_open (dpy, w,h, &w, &h, 9999,9999, 0, -1, 
		&handcolor, 
		&bgcolor,
		"fbclock", "", 
		FBUI_PROGTYPE_TOOL, false, false, -1, false,false,false, NULL,
		argc,argv);
	if (!win) 
		FATAL ("cannot create window");

	fbui_xpm_to_icon (dpy,win, clock_xpm);

	time_t t0 = time(0);
	while(1) {
		time_t t;
		bool need=false, mustclear=false;
		int size;
		usleep (250000);

		t = time(0);
		int tdiff = t - t0;
		if (tdiff >= 1) {
			need=true;
			mustclear=true;
			t0 = t;
		}

		if (!need) {
			Event ev;
			Window *win2;
			int err;

			if (err = fbui_poll_event (dpy, &ev,
			     		     FBUI_EVENTMASK_ALL & 
					     ~(FBUI_EVENTMASK_KEY | FBUI_EVENTMASK_MOTION)))
			{
				if (err != FBUI_ERR_NOEVENT)
					fbui_print_error (err);
				continue;
			}

			int num= ev.type;
printf ("%s got event %s\n", argv[0], fbui_get_event_name (ev.type));

			win2 = ev.win;
			if (win2 != win) {
printf ("ev.win=%08lx\n", (unsigned long)win2);
				FATAL ("event's window is not ours");
			}

			switch (num) {
			case FBUI_EVENT_EXPOSE:
				need=true;
				mustclear=false;
				break;
			case FBUI_EVENT_ENTER:
				continue;
			case FBUI_EVENT_LEAVE:
				continue;
			case FBUI_EVENT_MOVERESIZE:
				if (ev.key & FBUI_SIZE_CHANGED) {
					w = ev.width;
					h = ev.height;
					need=true;
				}
				break;
			}
		}

		if (!need) 
			continue;

		struct tm *tmp = localtime (&t);
		int hour = tmp->tm_hour;
		int minute = tmp->tm_min;
		int second = tmp->tm_sec;

		if (mustclear)
			fbui_clear (dpy, win);

		int shorter_side = w < h ? w : h;

		int deg;
		for (deg=0; deg < 360; deg += 6) {
			int x,y;
			getpos (deg, &x,&y, 5);
			
			int size=0;
			if ((deg % 90) == 0)
				size=2;
			else
			if ((deg % 30) == 0)
				size=1;
			fbui_fill_rect (dpy, win, x-size, y-size, x+size, y+size, markercolor);
		}

		int x,y;
		float p = minute;
		p /= 60.0;
		p *= 30;
		getpos ((hour%12)*30 + (int)p, &x,&y, shorter_side/4);
		fbui_draw_line (dpy, win, w/2,h/2,x,y, handcolor);
		fbui_draw_line (dpy, win, w/2+1,h/2,x+1,y, handcolor);
		fbui_draw_line (dpy, win, w/2,h/2+1,x+1,y+1, handcolor);
		
		getpos (minute*6, &x,&y, 15);
		fbui_draw_line (dpy, win, w/2,h/2,x,y, handcolor);
		
		getpos (second*6, &x,&y, 10);
		fbui_draw_point (dpy, win, x,y-2, gemcolor);
		fbui_draw_hline (dpy, win, x-1,x+1,y-1, gemcolor);
		fbui_draw_hline (dpy, win, x-2,x+2,y, gemcolor);
		fbui_draw_hline (dpy, win, x-1,x+1,y+1, gemcolor);
		fbui_draw_point (dpy, win, x,y+2, gemcolor);
		
		size=3;
		fbui_fill_rect (dpy, win, w/2-size, h/2-size, w/2+size, h/2+size, centercolor);

		fbui_flush (dpy, win);
	}

	fbui_window_close(dpy, win);
	fbui_display_close(dpy);
	return 0;
}
