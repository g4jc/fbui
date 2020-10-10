
/*=========================================================================
 *
 * fbterm, a terminal emulator based on ggiterm for FBUI.
 * Copyright (C) 2004 Zachary Smith, fbui@comcast.net
 * Portions from ggiterm are Copyright (C) by Aurelien Reynaud.
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
 *------------------------------------------------------------------------
 * Changes:
 *
 * My changes:
 *
 * 15 Oct 2004, fbui@comcast.net: added buffers for expose events
 * 19 Oct 2004, fbui@comcast.net: added -c and -geo support
 * 02 Nov 2004, fbui@comcast.net: updated for multi-window FBUI
 * 15 Sep 2005, fbui@comcast.net: updated some Erase routines.
 *
 *=======================================================================*/

#include <linux/fb.h>

#include "libfbui.h"
#include "libfbuiimage.h"
#include "fbterm.h"

#define BLINK_TIME 500

static Display *dpy = NULL;
static Window *win = NULL;
static Font *font = NULL;

extern struct winsize ws;
extern int master_fd;
extern int terminal_width, terminal_height;
static Image *image;


int default_bgcolor, default_fgcolor, fgcolor, bgcolor;
int blink_mode = 0, bold_mode = 0, invisible_mode = 0, reverse_mode = 0, underline_mode = 0, altcharset_mode = 0;
int autowrap = 1, force_cursor_mode = 0;
int region_top, region_bottom;
extern int linewrap_pending;

extern int (*fbtermPutShellChar) (unsigned char *, size_t *, wchar_t);

int vis_w, vis_h;
int cursor_x, cursor_y, cursor_x0, cursor_y0, cell_w, cell_h;
static RGB color[16] = {
	0,
	0xb00000,
	0xb000,
	0xb0b000,
	0xb0,
	0xb000b0,
	0xb0b0,
	0xb0b0b0,

	0,
	0xff0000,
	0xff00,
	0xffff00,
	0xff,
	0xff00ff,
	0xffff,
	0xffffff,
};

/* XPM */
static char * term_xpm[] = {
"24 32 571 2",
"  	c #000000",
". 	c #020302",
"+ 	c #070607",
"@ 	c #0B0B0A",
"# 	c #0F0F0F",
"$ 	c #131313",
"% 	c #171717",
"& 	c #1B1B1B",
"* 	c #1F1F1F",
"= 	c #232323",
"- 	c #282728",
"; 	c #2C2C2B",
"> 	c #303030",
", 	c #343434",
"' 	c #383838",
") 	c #3D3C3C",
"! 	c #414140",
"~ 	c #454445",
"{ 	c #494948",
"] 	c #4C4D4D",
"^ 	c #515151",
"/ 	c #555555",
"( 	c #040304",
"_ 	c #080808",
": 	c #0C0C0C",
"< 	c #101111",
"[ 	c #141515",
"} 	c #191818",
"| 	c #1C1D1C",
"1 	c #212121",
"2 	c #252524",
"3 	c #292929",
"4 	c #2D2D2D",
"5 	c #313232",
"6 	c #363535",
"7 	c #393939",
"8 	c #3D3E3D",
"9 	c #424242",
"0 	c #464646",
"a 	c #4A4A4A",
"b 	c #4E4E4E",
"c 	c #525353",
"d 	c #575756",
"e 	c #5B5B5B",
"f 	c #010101",
"g 	c #050505",
"h 	c #0A0A0A",
"i 	c #0E0D0D",
"j 	c #121212",
"k 	c #151616",
"l 	c #1A1A19",
"m 	c #1E1E1E",
"n 	c #222222",
"o 	c #272626",
"p 	c #2B2A2A",
"q 	c #2F2F2E",
"r 	c #333332",
"s 	c #373737",
"t 	c #3B3C3B",
"u 	c #3F3F3F",
"v 	c #444443",
"w 	c #474848",
"x 	c #4B4B4C",
"y 	c #504F50",
"z 	c #545454",
"A 	c #585858",
"B 	c #5C5C5C",
"C 	c #606061",
"D 	c #070706",
"E 	c #171718",
"F 	c #201F20",
"G 	c #282828",
"H 	c #2C2C2C",
"I 	c #393839",
"J 	c #3C3C3D",
"K 	c #414041",
"L 	c #454545",
"M 	c #494949",
"N 	c #4D4D4D",
"O 	c #565556",
"P 	c #595959",
"Q 	c #5E5D5E",
"R 	c #626261",
"S 	c #666666",
"T 	c #0D0C0C",
"U 	c #101011",
"V 	c #151515",
"W 	c #191819",
"X 	c #1D1D1D",
"Y 	c #252525",
"Z 	c #29292A",
"` 	c #323231",
" .	c #363536",
"..	c #3A3939",
"+.	c #3E3D3E",
"@.	c #4A4B4B",
"#.	c #4E4F4F",
"$.	c #535253",
"%.	c #575757",
"&.	c #5F5F5F",
"*.	c #646363",
"=.	c #686867",
"-.	c #6B6C6C",
";.	c #121112",
">.	c #161616",
",.	c #1A1A1B",
"'.	c #2B2A2B",
").	c #2F2F2F",
"!.	c #333333",
"~.	c #3C3B3B",
"{.	c #403F40",
"].	c #434343",
"^.	c #484847",
"/.	c #4C4C4C",
"(.	c #505050",
"_.	c #595859",
":.	c #5D5D5C",
"<.	c #616060",
"[.	c #656565",
"}.	c #696969",
"|.	c #6D6D6D",
"1.	c #717171",
"2.	c #1C1B1B",
"3.	c #202020",
"4.	c #242424",
"5.	c #2D2C2D",
"6.	c #303130",
"7.	c #343435",
"8.	c #454544",
"9.	c #4E4D4D",
"0.	c #555655",
"a.	c #5A595A",
"b.	c #5E5E5E",
"c.	c #626262",
"d.	c #6A6A6A",
"e.	c #6F6E6E",
"f.	c #727272",
"g.	c #777777",
"h.	c #292A29",
"i.	c #2D2D2E",
"j.	c #323232",
"k.	c #363636",
"l.	c #3A3A3A",
"m.	c #3E3E3E",
"n.	c #464747",
"o.	c #4F4E4F",
"p.	c #535353",
"q.	c #636463",
"r.	c #676867",
"s.	c #6B6B6C",
"t.	c #706F6F",
"u.	c #747474",
"v.	c #797878",
"w.	c #7C7C7C",
"x.	c #222322",
"y.	c #272727",
"z.	c #2B2B2B",
"A.	c #383737",
"B.	c #434344",
"C.	c #545554",
"D.	c #585859",
"E.	c #616161",
"F.	c #727171",
"G.	c #757675",
"H.	c #797A79",
"I.	c #7E7E7E",
"J.	c #828282",
"K.	c #2C2C2D",
"L.	c #343535",
"M.	c #393938",
"N.	c #3D3D3C",
"O.	c #414241",
"P.	c #454546",
"Q.	c #49494A",
"R.	c #515152",
"S.	c #555556",
"T.	c #5A5A5A",
"U.	c #676766",
"V.	c #6E6F6F",
"W.	c #737372",
"X.	c #7B7B7B",
"Y.	c #7F7F7F",
"Z.	c #848483",
"`.	c #878787",
" +	c #2E2E2E",
".+	c #373636",
"++	c #3E3F3F",
"@+	c #434243",
"#+	c #474646",
"$+	c #4B4B4B",
"%+	c #4F4F4F",
"&+	c #575857",
"*+	c #5B5C5B",
"=+	c #606060",
"-+	c #646463",
";+	c #686768",
">+	c #6C6C6C",
",+	c #707070",
"'+	c #747475",
")+	c #787878",
"!+	c #7D7C7D",
"~+	c #7A887A",
"{+	c #858585",
"]+	c #898989",
"^+	c #8D8D8D",
"/+	c #343334",
"(+	c #3B3C3C",
"_+	c #3F4040",
":+	c #444444",
"<+	c #484848",
"[+	c #4C4D4C",
"}+	c #595958",
"|+	c #5D5D5D",
"1+	c #6A696A",
"2+	c #6D6E6D",
"3+	c #767675",
"4+	c #7A7A7A",
"5+	c #3BC43B",
"6+	c #11F111",
"7+	c #0AF80A",
"8+	c #8B8A8A",
"9+	c #8E8E8E",
"0+	c #929293",
"a+	c #414142",
"b+	c #454645",
"c+	c #4E4E4D",
"d+	c #525152",
"e+	c #565656",
"f+	c #636363",
"g+	c #676666",
"h+	c #6F6F6F",
"i+	c #737373",
"j+	c #7F807F",
"k+	c #818681",
"l+	c #868985",
"m+	c #909090",
"n+	c #949494",
"o+	c #989898",
"p+	c #13DB13",
"q+	c #18D218",
"r+	c #0FEB0F",
"s+	c #19D619",
"t+	c #4C704C",
"u+	c #5B5B5C",
"v+	c #0FF00F",
"w+	c #0BF70B",
"x+	c #20D720",
"y+	c #677367",
"z+	c #608660",
"A+	c #10F110",
"B+	c #0EF30E",
"C+	c #6F936E",
"D+	c #24DF25",
"E+	c #0FF30F",
"F+	c #2EDA2E",
"G+	c #969595",
"H+	c #9A9A9A",
"I+	c #9E9E9E",
"J+	c #494848",
"K+	c #349434",
"L+	c #555654",
"M+	c #3D923D",
"N+	c #1CD71C",
"O+	c #626161",
"P+	c #548254",
"Q+	c #0CF50C",
"R+	c #677567",
"S+	c #25D426",
"T+	c #4DA64D",
"U+	c #40BB40",
"V+	c #51AE51",
"W+	c #24E124",
"X+	c #56B756",
"Y+	c #8E908E",
"Z+	c #56C055",
"`+	c #9C9C9B",
" @	c #9F9F9F",
".@	c #A3A3A3",
"+@	c #4A4949",
"@@	c #4E4D4E",
"#@	c #525F52",
"$@	c #5A5A5B",
"%@	c #5C635C",
"&@	c #666767",
"*@	c #6B6C6B",
"=@	c #18E518",
"-@	c #42B242",
";@	c #12EF12",
">@	c #26D926",
",@	c #25DC25",
"'@	c #749374",
")@	c #878888",
"!@	c #0BF80B",
"~@	c #8C948C",
"{@	c #949594",
"]@	c #919D92",
"^@	c #A0A1A1",
"/@	c #A5A5A5",
"(@	c #A9A9A9",
"_@	c #50504F",
":@	c #545453",
"<@	c #35A535",
"[@	c #5F625F",
"}@	c #3EA33E",
"|@	c #1FD81F",
"1@	c #3DB93D",
"2@	c #46B846",
"3@	c #19E919",
"4@	c #1BE81B",
"5@	c #8E8D8D",
"6@	c #27E227",
"7@	c #5ABF5A",
"8@	c #999B99",
"9@	c #5CC65C",
"0@	c #A7A6A6",
"a@	c #AAAAAA",
"b@	c #AEAFAE",
"c@	c #59595A",
"d@	c #23CE23",
"e@	c #0EF20E",
"f@	c #1FD91F",
"g@	c #608160",
"h@	c #767676",
"i@	c #659165",
"j@	c #719471",
"k@	c #44C343",
"l@	c #3CCE3C",
"m@	c #8F8F8F",
"n@	c #939393",
"o@	c #82A582",
"p@	c #29E529",
"q@	c #0EF60E",
"r@	c #28E828",
"s@	c #1FEE1F",
"t@	c #B0B0B0",
"u@	c #B4B4B4",
"v@	c #5B5A5A",
"w@	c #5F5E5F",
"x@	c #666966",
"y@	c #787777",
"z@	c #7B7C7C",
"A@	c #808080",
"B@	c #848484",
"C@	c #888888",
"D@	c #8C8C8C",
"E@	c #919091",
"F@	c #949595",
"G@	c #999998",
"H@	c #9C9D9C",
"I@	c #A1A1A1",
"J@	c #ADAEAD",
"K@	c #B2B2B2",
"L@	c #B6B5B6",
"M@	c #BABABA",
"N@	c #11ED11",
"O@	c #3DB53D",
"P@	c #757575",
"Q@	c #797979",
"R@	c #7D7D7E",
"S@	c #818181",
"T@	c #868585",
"U@	c #8A8A8A",
"V@	c #8D8E8E",
"W@	c #919192",
"X@	c #969696",
"Y@	c #A3A2A3",
"Z@	c #ABABAA",
"`@	c #AEAEAF",
" #	c #B3B3B3",
".#	c #B7B7B7",
"+#	c #BBBBBB",
"@#	c #C0C0C0",
"##	c #666665",
"$#	c #696A6A",
"%#	c #6E6D6D",
"&#	c #767677",
"*#	c #7A7A7B",
"=#	c #7E7F7E",
"-#	c #8B8B8B",
";#	c #8F908F",
">#	c #939394",
",#	c #989897",
"'#	c #9C9B9B",
")#	c #A4A4A4",
"!#	c #A8A8A8",
"~#	c #ACACAC",
"{#	c #B4B4B5",
"]#	c #B8B9B9",
"^#	c #BDBDBC",
"/#	c #C1C1C1",
"(#	c #C4C5C5",
"_#	c #6B6B6B",
":#	c #737474",
"<#	c #7B7B7C",
"[#	c #848584",
"}#	c #8D8C8C",
"|#	c #909190",
"1#	c #959595",
"2#	c #999999",
"3#	c #9D9D9D",
"4#	c #A1A1A2",
"5#	c #A6A6A5",
"6#	c #A9AAA9",
"7#	c #ADAEAE",
"8#	c #B2B1B2",
"9#	c #B6B6B6",
"0#	c #BEBEBE",
"a#	c #C2C2C2",
"b#	c #C7C7C6",
"c#	c #CBCBCA",
"d#	c #707170",
"e#	c #7D7D7D",
"f#	c #8A8989",
"g#	c #8E8D8E",
"h#	c #929292",
"i#	c #9B9B9B",
"j#	c #9E9E9F",
"k#	c #A6A6A7",
"l#	c #ABABAB",
"m#	c #AFAFAF",
"n#	c #B3B3B4",
"o#	c #B8B7B7",
"p#	c #BBBCBB",
"q#	c #C3C3C4",
"r#	c #C8C8C8",
"s#	c #CCCCCC",
"t#	c #D0D0D0",
"u#	c #7B7A7B",
"v#	c #7E7F7F",
"w#	c #838382",
"x#	c #8B8B8C",
"y#	c #8F8F90",
"z#	c #A0A0A0",
"A#	c #ADACAC",
"B#	c #B0B1B0",
"C#	c #B9B9B9",
"D#	c #BCBDBD",
"E#	c #C5C5C5",
"F#	c #C9CAC9",
"G#	c #CDCDCE",
"H#	c #D2D2D2",
"I#	c #D6D5D5",
"J#	c #7D7C7C",
"K#	c #808081",
"L#	c #858484",
"M#	c #888889",
"N#	c #8C8C8D",
"O#	c #919191",
"P#	c #9A9999",
"Q#	c #A1A2A1",
"R#	c #A6A5A6",
"S#	c #A9AAAA",
"T#	c #AEAEAE",
"U#	c #BBBABA",
"V#	c #BFBEBE",
"W#	c #C2C3C2",
"X#	c #C7C7C7",
"Y#	c #CBCBCB",
"Z#	c #CFCFCF",
"`#	c #D3D3D3",
" $	c #D7D7D7",
".$	c #DCDBDB",
"+$	c #828281",
"@$	c #868686",
"#$	c #9B9A9B",
"$$	c #A7A7A7",
"%$	c #B7B8B7",
"&$	c #BCBBBC",
"*$	c #C4C3C4",
"=$	c #D0D0D1",
"-$	c #D4D4D5",
";$	c #D9D9D9",
">$	c #DDDCDC",
",$	c #E0E1E1",
"'$	c #878788",
")$	c #949393",
"!$	c #9C9C9C",
"~$	c #A0A0A1",
"{$	c #A8A9A8",
"]$	c #ACADAC",
"^$	c #B0B1B1",
"/$	c #B5B4B4",
"($	c #B9B9B8",
"_$	c #BDBDBD",
":$	c #C1C2C1",
"<$	c #CECDCE",
"[$	c #D6D6D6",
"}$	c #DADADA",
"|$	c #DEDFDF",
"1$	c #E2E2E3",
"2$	c #E6E6E7",
"3$	c #9A9A99",
"4$	c #A2A1A1",
"5$	c #B2B3B2",
"6$	c #B6B7B6",
"7$	c #BFBEBF",
"8$	c #C3C3C3",
"9$	c #C6C7C7",
"0$	c #D7D7D8",
"a$	c #DBDCDC",
"b$	c #DFE0E0",
"c$	c #E4E4E4",
"d$	c #E8E8E8",
"e$	c #ECEDEC",
"f$	c #929392",
"g$	c #979796",
"h$	c #9E9F9F",
"i$	c #A7A7A8",
"j$	c #BCBCBC",
"k$	c #C5C4C4",
"l$	c #CDCCCD",
"m$	c #D1D1D1",
"n$	c #D5D5D5",
"o$	c #D9D8D9",
"p$	c #DDDDDC",
"q$	c #E1E1E1",
"r$	c #E6E5E5",
"s$	c #EAEAEA",
"t$	c #EDEEEE",
"u$	c #F2F2F1",
"v$	c #A1A0A0",
"w$	c #A4A5A4",
"x$	c #ADACAD",
"y$	c #B5B5B5",
"z$	c #B9B9BA",
"A$	c #BDBDBE",
"B$	c #CAC9CA",
"C$	c #CECECE",
"D$	c #D2D2D1",
"E$	c #D6D6D7",
"F$	c #DEDEDE",
"G$	c #E2E3E3",
"H$	c #E6E7E7",
"I$	c #EAEBEB",
"J$	c #EFEFEF",
"K$	c #F3F3F3",
"L$	c #F7F7F7",
"M$	c #A2A2A2",
"N$	c #A6A6A6",
"O$	c #BFBFBF",
"P$	c #CBCBCC",
"Q$	c #D0CFCF",
"R$	c #D3D4D3",
"S$	c #D8D7D7",
"T$	c #DCDCDC",
"U$	c #E0E0E0",
"V$	c #ECECEC",
"W$	c #F0F1F1",
"X$	c #F4F5F5",
"Y$	c #F8F9F9",
"Z$	c #FDFDFD",
"`$	c #A4A3A3",
" %	c #A7A8A7",
".%	c #ABACAB",
"+%	c #AFB0B0",
"@%	c #B8B8B8",
"#%	c #C1C0C0",
"$%	c #C4C5C4",
"%%	c #C9C8C8",
"&%	c #CCCDCD",
"*%	c #D0D1D0",
"=%	c #D4D5D5",
"-%	c #DDDDDD",
";%	c #E1E2E1",
">%	c #E5E5E5",
",%	c #EAEAE9",
"'%	c #F2F2F2",
")%	c #F6F6F6",
"!%	c #FAFAFA",
"~%	c #FEFEFE",
"{%	c #FFFFFF",
"      . + @ # $ % & * = - ; > , ' ) ! ~ { ] ^ / ",
"    ( _ : < [ } | 1 2 3 4 5 6 7 8 9 0 a b c d e ",
"f g h i j k l m n o p q r s t u v w x y z A B C ",
"D @ # $ E & F = G H > , I J K L M N ^ O P Q R S ",
"T U V W X 1 Y Z 4 `  ...+.9 0 @.#.$.%.e &.*.=.-.",
";.>.,.m n o '.).!.s ~.{.].^./.(.z _.:.<.[.}.|.1.",
"% 2.3.4.G 5.6.7.' J K 8.M 9.^ 0.a.b.c.S d.e.f.g.",
"X 1 Y h.i.j.k.l.m.9 n.@.o.p.%.e &.q.r.s.t.u.v.w.",
"x.y.z.).!.A.t {.B.^./.(.C.D.B E.[.}.|.F.G.H.I.J.",
"G K.> L.M.N.O.P.Q.b R.S.T.b.c.U.d.V.W.g.X.Y.Z.`.",
" +` .+l.++@+#+$+%+p.&+*+=+-+;+>+,+'+)+!+~+{+]+^+",
"/+A.(+_+:+<+[+(.z }+|+E.[.1+2+1.3+4+5+6+7+8+9+0+",
"7 8 a+b+Q.c+d+e+T.b.f+g+d.h+i+g.X.j+k+l+7+m+n+o+",
"u p+7+q+r+s+t+u+v+w+x+y+z+A+B+!+C+D+E+F+7+G+H+I+",
":+J+7+K+L+M+N+O+P+Q+R+S+T+U+V+J.W+X+Y+Z+7+`+ @.@",
"+@@@7+#@$@%@w+&@*@=@-@;@>@,@'@)@!@~@{@]@7+^@/@(@",
"_@:@7+<@[@}@|@>+,+1@Q+2@3@4@]+5@6@7@8@9@7+0@a@b@",
"/ c@7+d@e@f@g@f.h@i@7+j@k@l@m@n@o@p@q@r@7+s@t@u@",
"v@w@7+x@*@h+i+y@z@A@B@C@D@E@F@G@H@I@/@(@J@K@L@M@",
"<.N@7+7+O@P@Q@R@S@T@U@V@W@X@H+I+Y@0@Z@`@ #.#+#@#",
"##$#%#f.&#*#=#J.`.-#;#>#,#'# @)#!#~#t@{#]#^#/#(#",
"_#t.:#)+<#A@[#C@}#|#1#2#3#4#5#6#7#8#9#M@0#a#b#c#",
"d#P@Q@e#S@{+f#g#h#X@i#j#.@k#l#m#n#o#p#@#q#r#s#t#",
"&#u#v#w#`.x#y#n@,#`+z#)#!#A#B#u@C#D#/#E#F#G#H#I#",
"J#K#L#M#N#O#1#P#3#Q#R#S#T#K@9#U#V#W#X#Y#Z#`# $.$",
"+$@$U@9+0+X@#$ @Y@$$l#m#n#%$&$@#*$r#s#=$-$;$>$,$",
"'$x#m@)$o+!$~$)#{$]$^$/$($_$:$E#F#<$H#[$}$|$1$2$",
"^+O#1#3$3#4$R#a@T#5$6$U#7$8$9$c#Z#`#0$a$b$c$d$e$",
"f$g$i#h$.@i$l#t@u@o#j$@#k$r#l$m$n$o$p$q$r$s$t$u$",
"o+!$v$w${$x$^$y$z$A$/#E#B$C$D$E$}$F$G$H$I$J$K$L$",
"3#M$N$a@T#5$9#+#O$8$X#P$Q$R$S$T$U$c$d$V$W$X$Y$Z$",
"`$ %.%+%u@@%j$#%$%%%&%*%=%;$-%;%>%,%t$'%)%!%~%{%"};



void redraw (bool use_clip, 
	     short clip_x0, short clip_y0, short clip_x1, short clip_y1)
{
	int j;

	if (!use_clip) {
		Image_draw (image, dpy, win, 0,0);
	} else {
		Image_draw_partial (image, dpy, win,
			0,0, clip_x0, clip_y0, clip_x1, clip_y1);
	}
}

void resize (int new_width, int new_height)
{
	int i, j;

	if (new_width==terminal_width && new_height==terminal_height)
		return;

	Image_resize (image, new_width*cell_w, new_height*cell_h);
	Image_draw (image, dpy, win, 0,0);

	terminal_width = new_width;
	terminal_height = new_height;
}


void scroll_exposebuf_up (int num, int top, int bottom)
{
	Image_copy_area (image, 0, cell_h, image->width, image->height - cell_h,
			0,0);
	Image_fill_rect (image, 0, image->height-cell_h, image->width-1,
			image->height-1, color[bgcolor]);
	Image_draw (image, dpy, win, 0,0);
}


void scroll_exposebuf_down()
{
#if 0
	int i,j;
	for (j=terminal_height-2; j>=0; j--) {
		for (i=0; i<terminal_width; i++) {
			int ix = i+terminal_width*j;
			exposebuffer [ix+terminal_width] = exposebuffer[ix];
			exposebuffer_fg [ix+terminal_width] = exposebuffer_fg [ix];
			exposebuffer_bg [ix+terminal_width] = exposebuffer_bg [ix];
		}
	}
	for (i=0; i<terminal_width; i++) {
		exposebuffer [i] = ' ';
		exposebuffer_fg [i] = default_bgcolor;
		exposebuffer_bg [i] = default_bgcolor;
	}
#endif
}



void MoveCursor (int, int);


wchar_t AsciiGetAltChar (wchar_t charcode)
{
		switch (charcode)
		{
			case '+': charcode = '>'; break; /* vt100 '+': arrow pointing right */
			case ',': charcode = '<'; break; /* vt100 ',': arrow pointing left */
			case '-': charcode = 0136; break; /* vt100 '-': arrow pointing up */
			case '.': charcode = 'v'; break; /* vt100 '.': arrow pointing down */
			case '0': charcode = '#'; break; /* vt100 '0': solid square block */
			case '`': charcode = '+'; break; /* vt100 '`': diamond */
			case 'a': charcode = ':'; break; /* vt100 'a': checker board (stipple) */
			case 'f': charcode = '\\'; break; /* vt100 'f': degree */
			case 'g': charcode = '#'; break; /* vt100 'g': plus/minus */
			case 'h': charcode = '#'; break; /* vt100 'h': board os squares */
			case 'j': charcode = '+'; break; /* vt100 'j': rightdown corner */
			case 'k': charcode = '+'; break; /* vt100 'k': upright corner */
			case 'l': charcode = '+'; break; /* vt100 'l': upleft corner */
			case 'm': charcode = '+'; break; /* vt100 'm': downleft corner */
			case 'n': charcode = '+'; break; /* vt100 'n': crossover */
			case 'o': charcode = '~'; break; /* vt100 'o': scan line 1 */
			case 'p': charcode = '-'; break; /* vt100 'p': scan line 3 */
			case 'q': charcode = '-'; break; /* vt100 'q': horizontal line */
			case 'r': charcode = '-'; break; /* vt100 'r': scan line 7 */
			case 's': charcode = '_'; break; /* vt100 's': scan line 9 */
			case 't': charcode = '+'; break; /* vt100 't': tee pointing right */
			case 'u': charcode = '+'; break; /* vt100 'u': tee pointing left */
			case 'v': charcode = '+'; break; /* vt100 'v': tee pointing up */
			case 'w': charcode = '+'; break; /* vt100 'w': tee pointing down */
			case 'x': charcode = '|'; break; /* vt100 'x': vertical line */
			case 'y': charcode = '<'; break; /* vt100 'y': less or equal */
			case 'z': charcode = '>'; break; /* vt100 'z': more or equal */
			case '{': charcode = '*'; break; /* vt100 '{': Pi */
			case '|': charcode = '!'; break; /* vt100 '|': not equal */
			case '}': charcode = 'f'; break; /* vt100 '}': UK pound */
			case '~': charcode = 'o'; break; /* vt100 '~': bullet */
		}
		return charcode;
}


void
fbtermBlinkCursor (int force)
{
	int x,y;
	x = cell_w*(int)(cursor_x/cell_w);
	y = cell_h*(int)(cursor_y/cell_h);

	if (force != CURSOR_HIDE)
	{
		fbui_fill_rect (dpy, win,
			x, y, x + cell_w - 1, y + cell_h - 1, color[fgcolor]);
	}
	else
	{
		fbui_fill_rect (dpy, win,
			x, y, x + cell_w - 1, y + cell_h - 1, color[bgcolor]);
	}
	fbui_flush (dpy, win);
}

void
fbtermDeleteChars (unsigned int nb_chars)
{
	int x, y;
	
	x = cell_w * (int)(cursor_x / cell_w);
	y = cell_h * (int)(cursor_y / cell_h);

	Image_copy_area (image, x+(nb_chars*cell_w), y, 
		cell_w * nb_chars, cell_h, 
		x, y);
	Image_fill_rect (image, image->width - (nb_chars*cell_w),
			y, image->width-1, y+cell_h-1, 
			color[bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermInsertChars (unsigned int nb_chars)
{
	int x, y;
	
	x = cell_w * (int)(cursor_x / cell_w);
	y = cell_h * (int)(cursor_y / cell_h);

	Image_copy_area (image, x, y, vis_w-x, cell_h, x+(nb_chars*cell_w), y);
	Image_fill_rect (image, x, y, nb_chars*cell_w, cell_h, 
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseNChars (unsigned int nb_chars)
{
	int x, y;
	
	x = cell_w * (int)(cursor_x / cell_w);
	y = cell_h * (int)(cursor_y / cell_h);

	Image_fill_rect (image, x, y, x+nb_chars*cell_w-1, y+cell_h-1, 
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseToEOL ()
{
	int x, y;
	
	x = cell_w * (int)(cursor_x / cell_w);
	y = cell_h * (int)(cursor_y / cell_h);

	Image_fill_rect (image, x, y, image->width-1, 
		y + cell_h - 1, 
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseFromBOL ()
{
	int x, y;
	
	x = cell_w * (int)(cursor_x / cell_w + 1);
	y = cell_h * (int)(cursor_y / cell_h);

	Image_fill_rect (image, 0, y, x+cell_w-1, y+cell_h-1, 
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseLine ()
{
	int y;
	
	y = cell_h * (int)(cursor_y / cell_h);

#if 0
	int i = 0;
	while (i < terminal_width)
		exposebuffer [cursor_y*terminal_width + i++] = ' ';
#endif

	Image_fill_rect (image, 0, y, 
		image->width-1, y+cell_h-1, 
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseToEOD ()
{
	int y;
	
	fbtermEraseToEOL ();
	y = cell_h * cursor_y;

#if 0
	int i = cursor_y*terminal_width + cursor_x;
	while (i < terminal_width*terminal_height)
		exposebuffer [i++] = ' ';
#endif

	Image_fill_rect (image, 0, y+cell_h-1, 
		image->width-1, image->height-1,
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseFromBOD ()
{
	int y;
	
	fbtermEraseFromBOL ();
	y = cell_h * (int)(cursor_y / cell_h);

	Image_fill_rect (image, 0, 0,
		image->width-1, y-1,
		color[default_bgcolor]);
	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermEraseDisplay ()
{
	int i;

	Image_fill_rect (image, 0, 0,
		image->width-1, image->height-1,
		color[default_bgcolor]);

	Image_draw (image, dpy, win, 0,0); // XX use partial
}

void
fbtermScrollUp (unsigned int nb_lines)
{
	scroll_exposebuf_up (nb_lines, region_top, region_bottom);
}

void
fbtermScrollDown (unsigned int nb_lines)
{
	scroll_exposebuf_down();
}

void
fbtermNextPos ()
{
	MoveCursor (cursor_x + cell_w, cursor_y);
}


int
FBUIWriteChar (wchar_t charcode)
{
	int cur_bgcolor, cur_fgcolor, dummy_color;

	if (linewrap_pending)
	{
		MoveCursor (cursor_x0, cursor_y + cell_h);
	}
	cur_bgcolor = bgcolor;
	cur_fgcolor = fgcolor;
	if (altcharset_mode)
	{
		charcode = AsciiGetAltChar (charcode);
	}
	if (reverse_mode)
	{
		dummy_color = cur_bgcolor;
		cur_bgcolor = cur_fgcolor;
		cur_fgcolor = dummy_color;
	}
	if (invisible_mode)
	{
		cur_fgcolor = cur_bgcolor;
	}
	if (bold_mode)
	{
		cur_fgcolor += 8;
	}

	Image_fill_rect (image, cursor_x, cursor_y, cursor_x + cell_w - 1, cursor_y + cell_h - 1, color[cur_bgcolor]);
	char s[2] = { (char)charcode, 0 };

	Image_draw_string (image, font, cursor_x, cursor_y, s, color[cur_fgcolor]);

	if (underline_mode)
		Image_fill_rect (image, 
			cursor_x, cursor_y+cell_h-1,
			cursor_x+cell_w-1, cursor_y+cell_h-1,
			color[cur_fgcolor]);

	Image_draw_partial (image, dpy, win, 0,0, cursor_x, cursor_y, cursor_x+cell_w-1, cursor_y+cell_h-1);

	return 0;
}


void
HandleFBUIEvents (unsigned char *shellinput, size_t *shellinput_size)
{
	struct timeval tv;
	int i;

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	Event ev;
	unsigned char event_num;

	char check_again = false;

	usleep (10000);

	do {
		if (fbui_poll_event (dpy, &ev, FBUI_EVENTMASK_ALL))
			return;

		event_num = ev.type;

		if (ev.win != win) {
			FATAL ("event not for fbterm window");
		}

		if (event_num == FBUI_EVENT_EXPOSE) {
			if (!ev.has_rects)
				redraw(false,0,0,0,0);
			else {
				struct fbui_rects *r = &ev.rects;
				short min_x = 10000;
				short min_y = 10000;
				short max_x = -10000;
				short max_y = -10000;
				int ix = 0;
				int lim = r->total << 2;
				short x0,y0,x1,y1;
				while (ix < lim) {
					x0 = r->c[ix++];
					y0 = r->c[ix++];
					x1 = r->c[ix++];
					y1 = r->c[ix++];
					if (x0 < min_x)
						min_x = x0;
					if (y0 < min_y)
						min_y = y0;
					if (x1 > max_x)
						max_x = x1;
					if (y1 > max_y)
						max_y = y1;
					//redraw (true, x0,y0,x1,y1);
				}
				redraw (true, min_x, min_y, max_x, max_y);
			}

			// If a window is being moved over the fbterm,
			// exposures will come in rapid succession,
			// thus we ought to check again for one right away.
			check_again = true;
		}
		else if (event_num == FBUI_EVENT_MOVERESIZE) {

			if (ev.key & FBUI_SIZE_CHANGED) {
				vis_w = ev.width;
				vis_h = ev.height;
				int new_width = vis_w / cell_w;
				int new_height = vis_h / cell_h;
				resize (new_width, new_height);

				// update the OS
				ws.ws_col = terminal_width;
				ws.ws_row = terminal_height;
				ws.ws_xpixel = vis_w;
				ws.ws_ypixel = vis_h;
				if (ioctl (master_fd, TIOCSWINSZ, &ws) < 0) {
					WARNING("unable to set tty window size");
				}
			}

			if (ev.key & FBUI_CONTENTS_CHANGED)
				redraw (false,0,0,0,0);
		}
		else if (event_num == FBUI_EVENT_ENTER) {
			printf ("fbterm got Enter\n");
			return;
		}
		else if (event_num == FBUI_EVENT_LEAVE) {
			printf ("fbterm got Leave\n");
			return;
		}
		else if (event_num == FBUI_EVENT_ACCEL) {
			// Not using accelerators
			return;
		}
		else if (event_num == FBUI_EVENT_KEY) {
			short ch = fbui_convert_key (dpy, ev.key);
			if (!ch) 
				return;

			if (SHELLINPUT_SIZE - *shellinput_size >= (MB_CUR_MAX>3?MB_CUR_MAX:3))
			switch (ch) {
			case '\n':
				shellinput[*shellinput_size] = '\n';
				(*shellinput_size)++;
				break;

			case 8:
			case 9:
			case FBUI_DEL:
				shellinput[*shellinput_size] = 0x08;
				(*shellinput_size)++;
				break;

			default:
				fbtermPutShellChar (shellinput, shellinput_size, ch);
				break;

			case FBUI_F1:
				memcpy (shellinput+*shellinput_size, "\033OP", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F2:
				memcpy (shellinput+*shellinput_size, "\033OQ", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F3:
				memcpy (shellinput+*shellinput_size, "\033OR", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F4:
				memcpy (shellinput+*shellinput_size, "\033OS", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F5:
				memcpy (shellinput+*shellinput_size, "\033Ot", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F6:
				memcpy (shellinput+*shellinput_size, "\033Ou", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F7:
				memcpy (shellinput+*shellinput_size, "\033Ov", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F8:
				memcpy (shellinput+*shellinput_size, "\033Ol", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F9:
				memcpy (shellinput+*shellinput_size, "\033Ow", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_F10:
				memcpy (shellinput+*shellinput_size, "\033Ox", 3);
				(*shellinput_size) += 3;
				break;

			case FBUI_UP:
				memcpy (shellinput+*shellinput_size, "\033OA", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_DOWN:
				memcpy (shellinput+*shellinput_size, "\033OB", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_LEFT:
				memcpy (shellinput+*shellinput_size, "\033OD", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_RIGHT:
				memcpy (shellinput+*shellinput_size, "\033OC", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_PGUP:
				memcpy (shellinput+*shellinput_size, "\033Os", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_PGDN:
				memcpy (shellinput+*shellinput_size, "\033On", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_INS:
				memcpy (shellinput+*shellinput_size, "\033[L", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_HOME:
				memcpy (shellinput+*shellinput_size, "\033Oq", 3);
				(*shellinput_size) += 3;
				break;
			case FBUI_END:
				memcpy (shellinput+*shellinput_size, "\033Op", 3);
				(*shellinput_size) += 3;
				break;
			}
		}
	} while (check_again);
}


void FBUIMapColors ()
{
	int i;
}

int
FBUIInit_graphicsmode (int vc,short cols,short rows,short xrel,short yrel)
{
	int err;
	char dummy[64];
	long size = rows * cols;

	terminal_width = cols;
	terminal_height = rows;

printf ("fbterm: cols=%d rows=%d\n",cols,rows);

	default_fgcolor = 7;
	default_bgcolor = 0;
	fgcolor = default_fgcolor;
	bgcolor = default_bgcolor;

	int argc=1;
	char *argv[1] = {"foo"};

	if (xrel > 999) {
		xrel = 2;
		yrel = 15;
	}

        short win_w, win_h;

	long fg;
	long bg = color[default_bgcolor];

	dpy = fbui_display_open ();
        if (!dpy)
                FATAL ("cannot open display");
	
        font = Font_new ();
        if (!pcf_read (font, "courR12.pcf")) {
                Font_free (font);
		FATAL ("cannot load font");
        }

	cell_h = font->ascent + font->descent;

	if (!cell_h) {
		FATAL ("oops, invalid font data\n");
	}
	
	cell_w = font->widths [' ' - font->first_char];

	if (!cell_w) {
		cell_w = font->widths ['W' - font->first_char];
		if (!cell_w) {
			FATAL ("oops, invalid font data\n");
		}
	}

	int default_w = cell_w * cols;
	int default_h = cell_h * rows;
printf ("fbterm: trying for window size %d,%d to provide %dx%d text\n", default_w, default_h, rows,cols);

        win = fbui_window_open (dpy, default_w, default_h, 
		&win_w, &win_h,
		9999,9999,
		xrel, yrel,
		&fg, &bg, 
		"fbterm", "", 
		FBUI_PROGTYPE_APP, 
		false,false, 
		vc,
		true, 
		false,
		false, 
		NULL,
		argc,argv);
        if (!win)
                FATAL ("cannot create window");

	fbui_xpm_to_icon (dpy,win,term_xpm);

	image = Image_new(IMAGE_RGB, win_w, win_h, 24);

	vis_w = win_w;
	vis_h = win_h;

	if (win_w < 1 || win_h < 1) {
		printf ("Don't have window dimensions yet.\n");
	} else {
		printf ("Window dimensions %d,%d (%dx%d) text\n", 
			win_w,win_h,win_w/cell_w,win_h/cell_h);
	}

	cursor_x0 = 0;
	cursor_y0 = 0;

	return 0;
}


#ifdef DEBUG
void ShowGrid (void)
{
	int x, y;
	
	ggiSetGCForeground (vis, color[2]);
	for (x = 0; x < vis_w; x += cell_w)
	{
		ggiDrawVLine (vis, x, 0, vis_h);
	}
	for (y = 0; y < vis_h; y += cell_h)
	{
		ggiDrawHLine (vis, 0, y, vis_w);
	}
}
#endif /* DEBUG */

void FBUIExit (void)
{
	if (image)
		Image_delete (image);

	fbui_window_close (dpy, win);
	fbui_display_close (dpy);
}
