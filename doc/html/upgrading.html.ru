<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html><head><meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1"><title>&#1055;&#1077;&#1088;&#1077;&#1093;&#1086;&#1076; &#1089;  mod_dtcl &#1080;&#1083;&#1080; NeoWebScript (NWS)</title><link rel="stylesheet" href="rivet.css" type="text/css"><meta name="generator" content="DocBook XSL Stylesheets V1.60.1"><link rel="home" href="index.html.ru" title="Apache Rivet"><link rel="up" href="index.html.ru" title="Apache Rivet"><link rel="previous" href="internals.html.ru" title="&#1042;&#1085;&#1091;&#1090;&#1088;&#1077;&#1085;&#1085;&#1086;&#1089;&#1090;&#1080; Rivet"></head><body bgcolor="white" text="black" link="#0000FF" vlink="#840084" alink="#0000FF"><div class="navheader"><table width="100%" summary="Navigation header"><tr><th colspan="3" align="center">&#1055;&#1077;&#1088;&#1077;&#1093;&#1086;&#1076; &#1089;  mod_dtcl &#1080;&#1083;&#1080; NeoWebScript (NWS)</th></tr><tr><td width="20%" align="left"><a accesskey="p" href="internals.html.ru"><img src="images/prev.png" alt="Prev"></a>�</td><th width="60%" align="center">�</th><td width="20%" align="right">�</td></tr></table></div><div class="section" lang="en"><div class="titlepage"><div><div><hr><h2 class="title" style="clear: both"><a name="upgrading"></a>&#1055;&#1077;&#1088;&#1077;&#1093;&#1086;&#1076; &#1089;  mod_dtcl &#1080;&#1083;&#1080; NeoWebScript (NWS)</h2></div></div><div></div></div><p style="width:90%">
      &#1054;&#1073;&#1088;&#1072;&#1090;&#1085;&#1072;&#1103; &#1089;&#1086;&#1074;&#1084;&#1077;&#1089;&#1090;&#1080;&#1084;&#1086;&#1089;&#1090;&#1100; &#1085;&#1077; &#1073;&#1099;&#1083;&#1072; &#1085;&#1080;&#1082;&#1086;&#1075;&#1076;&#1072; &#1086;&#1089;&#1086;&#1073;&#1086;&#1081; &#1094;&#1077;&#1083;&#1100;&#1102; &#1088;&#1072;&#1079;&#1088;&#1072;&#1073;&#1086;&#1090;&#1095;&#1080;&#1082;&#1086;&#1074;,
      &#1085;&#1086; &#1084;&#1099; &#1077;&#1077; &#1074;&#1089;&#1077; &#1088;&#1072;&#1074;&#1085;&#1086; &#1087;&#1088;&#1077;&#1076;&#1086;&#1089;&#1090;&#1072;&#1074;&#1083;&#1103;&#1077;&#1084; &#1087;&#1086; &#1080;&#1089;&#1090;&#1086;&#1088;&#1080;&#1095;&#1077;&#1089;&#1082;&#1080;&#1084; &#1087;&#1088;&#1080;&#1095;&#1080;&#1085;&#1072;&#1084; &#1080; 
      &#1089; &#1091;&#1095;&#1077;&#1090;&#1086;&#1084; &#1087;&#1086;&#1078;&#1077;&#1083;&#1072;&#1085;&#1080;&#1081; &#1090;&#1077;&#1093; &#1082;&#1090;&#1086; &#1087;&#1077;&#1088;&#1077;&#1082;&#1083;&#1102;&#1095;&#1072;&#1077;&#1090;&#1089;&#1103; &#1085;&#1072; Rivet &#1089; mod_dtcl &#1080; NeoWebScript (NWS).
    </p><div class="section" lang="en"><div class="titlepage"><div><div><h3 class="title"><a name="id5397070"></a>mod_dtcl</h3></div></div><div></div></div><p style="width:90%">
        Rivet &#1073;&#1072;&#1079;&#1080;&#1088;&#1091;&#1077;&#1090;&#1089;&#1103; &#1085;&#1072; &#1082;&#1086;&#1076;&#1077; mod_dtcl, &#1085;&#1086; &#1076;&#1086;&#1089;&#1090;&#1072;&#1090;&#1086;&#1095;&#1085;&#1086; &#1093;&#1086;&#1088;&#1086;&#1096;&#1086; &#1087;&#1077;&#1088;&#1077;&#1088;&#1072;&#1073;&#1086;&#1090;&#1072;&#1085;. &#1054;&#1089;&#1085;&#1086;&#1074;&#1085;&#1072;&#1103;
        &#1080;&#1076;&#1077;&#1103; &#1086;&#1089;&#1090;&#1072;&#1083;&#1072;&#1089;&#1100; &#1090;&#1072; &#1078;&#1077;, &#1085;&#1086; &#1073;&#1086;&#1083;&#1100;&#1096;&#1086;&#1077; &#1082;&#1086;&#1083;&#1080;&#1095;&#1077;&#1090;&#1074;&#1086; &#1082;&#1086;&#1084;&#1072;&#1085;&#1076; &#1080;&#1079;&#1084;&#1077;&#1085;&#1080;&#1083;&#1086;&#1089;&#1100;.
      </p></div><div class="section" lang="en"><div class="titlepage"><div><div><h3 class="title"><a name="id5397093"></a>NeoWebScript</h3></div></div><div></div></div><p style="width:90%">&#1055;&#1086;&#1090;&#1086;&#1084;.</p></div></div><div class="navfooter"><hr><table width="100%" summary="Navigation footer"><tr><td width="40%" align="left"><a accesskey="p" href="internals.html.ru"><img src="images/prev.png" alt="Prev"></a>�</td><td width="20%" align="center"><a accesskey="u" href="index.html.ru"><img src="images/up.png" alt="Up"></a></td><td width="40%" align="right">�</td></tr><tr><td width="40%" align="left" valign="top">&#1042;&#1085;&#1091;&#1090;&#1088;&#1077;&#1085;&#1085;&#1086;&#1089;&#1090;&#1080; Rivet�</td><td width="20%" align="center"><a accesskey="h" href="index.html.ru"><img src="images/home.png" alt="Home"></a></td><td width="40%" align="right" valign="top">�</td></tr></table></div></body></html>