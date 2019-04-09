#!/usr/bin/perl

#
# t2h.pl - a simple texi2html converter which also generates the files needed for chm creation.
#
# Written by
#  Peter Verhas
#
# Modified for VICE by
#  Marco van den Heuvel <blackystardust68@yahoo.com>
#
# This file is part of VICE, the Versatile Commodore Emulator.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#
# Usage: t2h.pl <texi-file>
#                $1
#

sub getitoc {
  my $p = shift;
  my $i,$r;
  return $ITOC{$p} if defined $ITOC{$p};
  $r = $p;
  $p = lc $p;
  $p =~ s/\W//g;
  return $ITOC{$p} if defined $ITOC{$p};

  for $i (@TOCI){
    my $k = lc $TOC{$i};
    $k =~ s/\W//;
    return $i if $k eq $p;
    }

  $p = lc $r;
  $p =~ s/\s.*$//;
  $p =~ s/\W.*$//;
  for $i (@TOCI){
    my $k = lc $TOC{$i};
    $k =~ s/\s.*$//;
    $k =~ s/\W.*$//;
    return $i if $k eq $p;
    }

  return undef;
  }

sub rtfizee {
  my $line = shift;
  my $filext = shift;

  $filext = 'html' unless defined $filext;

  $line =~ s[\@\@][SAVEALLDOUBLESOBAKA]g;
  $line =~ s[\\][SAVEALLDBACKSLASH]g;


  $line =~ s[\@file\{(.*?)\}][\`\{\\f1\\fs18 $1\}\']g;
  $line =~ s[\@code\{(.*?)\}][\{\\f1\\fs18 $1\}]g;
  $line =~ s[\@var\{(.*?)\}][\{\\f1\\fs18 $1\}]g;
  $line =~ s[\@command\{(.*?)\}][\{\\f1\\fs18 $1\}]g;
  $line =~ s[\@acronym\{(.*?)\}][\{\\f1\\fs18 $1\}]g;
  $line =~ s[\@b\{(.*?)\}][\{\\b\\f0\\fs18 $1\}]g;
  $line =~ s[\@strong\{(.*?)\}][\{\\i\\f0\\fs18 $1\}]g;
  $line =~ s[\@emph\{(.*?)\}][\{\\i\\f0\\fs18 $1\}]g;
  $line =~ s[\@option\{(.*?)\}][\`{\\f1\\fs18 $1\}\']g;
  $line =~ s[\@itemize][]g;
  $line =~ s[\@end\s+itemize][]g;
  $line =~ s[\@item][]g;
  $line =~ s[\@itemx][]g;
  $line =~ s[\@\{][\\\{]g;
  $line =~ s[\@\}][\\\}]g;
  $line =~ s[\@\$][\$]g;

  my @linarr = split /\n/ , $line;
  my $verbatim = 0;
  for $L ( @linarr ){
    if( $L =~ /\@example/ ){
      if( $verbatim ){
        print $errs;
        die "Doubled verbatim";
        }
      $errs = '';
      $verbatim = 1;
      $L = "\{\\f1\\fs18";
      next;
      }
    if( $L =~ /\@end\s+example/ ){
      if( ! $verbatim ){
        die "Unopened verbatim";
        }
      $L = "\}\n";
      $verbatim = 0;
      next;
      }
    if( $verbatim ){
      $errs .= $L . "\n";
      $L = "$L\\par";
      }
    }

  if( $verbatim ){
    die "unclosed verbatim";
    }
  $line = join("\n",@linarr);

  if( $filext eq '#' ){
    while( $line =~ m[\@xref\{(.*?)\}] ){
      my $itoc = &getitoc($1);
      $line =~ s[\@xref\{(.*?)\}][$1];
      }
    }else{
    while( $line =~ m[\@xref\{(.*?)\}] ){
      my $itoc = &getitoc($1);
      $line =~ s[\@xref\{(.*?)\}][$1];
      }
    }
  $line =~ s[\@uref\{(.*?),(.*?)\}][$2]g;
  $line =~ s[\@uref\{(.*?)\}][$1]g;
  $line =~ s[\@email\{(.*?)\}][\{\\f1\\fs18 $1\}]g;

  $line =~ s[SAVEALLDOUBLESOBAKA][\@]g;
  $line =~ s[SAVEALLDBACKSLASH][\\\\]g;
  $line =~ s/\n\n/\n\\par\\par\n/g;

  $line;
  }

sub htmlizee {
  my $line = shift;
  my $filext = shift;

  $filext = 'html' unless defined $filext;

  $line =~ s[\@\@][SAVEALLDOUBLESOBAKA]g;

  $line =~ s[\&][\&amp;]g;
  $line =~ s[\<][\&lt;]g;
  $line =~ s[\>][\&gt;]g;

  $line =~ s[\@file\{(.*?)\}][\`<font size=\"3\"><tt>$1</tt></font>\']g;
  $line =~ s[\@code\{(.*?)\}][<font size=\"3\"><tt>$1</tt></font>]g;
  $line =~ s[\@var\{(.*?)\}][<font size=\"3\"><tt>$1</tt></font>]g;
  $line =~ s[\@command\{(.*?)\}][<font size=\"3\"><tt>$1</tt></font>]g;
  $line =~ s[\@acronym\{(.*?)\}][<font size=\"3\"><tt>$1</tt></font>]g;
  $line =~ s[\@b\{(.*?)\}][<B>$1</B>]g;
  $line =~ s[\@strong\{(.*?)\}][<I>$1</I>]g;
  $line =~ s[\@emph\{(.*?)\}][<I>$1</I>]g;
  $line =~ s[\@option\{(.*?)\}][\`<font size=\"3\"><tt>$1</tt></font>\']g;
  $line =~ s[\@samp\{(.*?)\}][\`<SAMP>$1</SAMP>\']g;
  $line =~ s[\@dfn\{(.*?)\}][<EM>$1</EM>]g;
  $line =~ s[\@key\{(.*?)\}][<KBD>$1</KBD>]g;
  $line =~ s[\@kbd\{(.*?)\}][<KBD>$1</KBD>]g;
  $line =~ s[\@unnumberedsec(.*?)\n][<H2>$1</H2>\n]g;
  $line =~ s[\@copyright\{\}][\&copy;]g;
  $line =~ s[\@itemize][<UL>]g;
  $line =~ s[\@end\s+itemize][</UL>]g;
  $line =~ s[\@enumerate(.*?)\n][<OL>]g;
  $line =~ s[\@end\s+enumerate][</OL>]g;
  $line =~ s[\@itemx][<LI>]g;
  $line =~ s[\@item][<LI>]g;
  $line =~ s[\@heading(.*?)\n][<P><STRONG>$1</STRONG></P>\n]g;
  $line =~ s[\@example][<FONT SIZE="3" COLOR="BLUE"><PRE>]g;
  $line =~ s[\@end\s+example][</PRE></FONT>]g;
  $line =~ s[\@display][<PRE>]g;
  $line =~ s[\@end\s+display][</PRE>]g;
  $line =~ s[\@smallexample][<PRE>]g;
  $line =~ s[\@end\s+smallexample][</PRE>]g;
  $line =~ s[\@multitable(.*?)\n][<TABLE BORDER><TR><TD>$1</TD>]g;
  $line =~ s[\@end\s+multitable][</TR></TABLE>]g;
  $line =~ s[\@table][]g;
  $line =~ s[\@tab(.*?)\n][<TD>$1</TD>]g;
  $line =~ s[\@rowstart(.*?)\n][</TR>\n<TR><TD>$1</TD>]g;
  $line =~ s[\@dots\{\}][...]g;
  $line =~ s[\@\{][\{]g;
  $line =~ s[\@\}][\}]g;
  $line =~ s[\@\$][\$]g;

# ignored items
  $line =~ s[\@center][]g;
  $line =~ s[\@iftex][]g;
  $line =~ s[\@end\s+iftex][]g;
  $line =~ s[\@ifset(.*?)\n][]g;
  $line =~ s[\@end\s+ifset][]g;
  $line =~ s[\@ifclear(.*?)\n][]g;
  $line =~ s[\@end\s+ifclear][]g;
  $line =~ s[\@group][]g;
  $line =~ s[\@end\s+group][]g;
  $line =~ s[\@end\s+table][]g;
  $line =~ s[\@vskip(.*?)\n][]g;
  $line =~ s[\@page][]g;
  $line =~ s[\@bullet][]g;
  $line =~ s[\@code][]g;

  if( $filext eq '#' ){
    while( $line =~ m[\@xref\{(.*?)\}] ){
      my $itoc = &getitoc($1);
      $line =~ s[\@xref\{(.*?)\}][See <a href="#$itoc">$1</A>];
      }
    }else{
    while( $line =~ m[\@xref\{(.*?)\}] ){
      my $itoc = &getitoc($1);
      $line =~ s[\@xref\{(.*?)\}][See <a href="${infile}_$itoc${filext}">$1</A>];
      }
    }

  if( $filext eq '#' ){
    while( $line =~ m[\@pxref\{(.*?)\}] ){
      my $itoc = &getitoc($1);
      $line =~ s[\@pxref\{(.*?)\}][see <a href="#$itoc">$1</A>];
      }
    }else{
    while( $line =~ m[\@pxref\{(.*?)\}] ){
      my $itoc = &getitoc($1);
      $line =~ s[\@pxref\{(.*?)\}][see <a href="${infile}_$itoc${filext}">$1</A>];
      }
    }
  $line =~ s[\@uref\{(.*?),(.*?)\}][<a href="$1">$2</A>]g;
  $line =~ s[\@uref\{(.*?)\}][<a href="$1">$1</A>]g;
  $line =~ s[\@email\{(.*?)\}][<a href="mailto:$1">$1</A>]g;

  $line =~ s[SAVEALLDOUBLESOBAKA][\@]g;
  $line =~ s/\n\n/\n\<P\>\n/g;

  $line;
  }

$infile = shift;
$outfile = shift;

open(F,$infile) or die "Can not open input file $infile";
@lines = <F>;
close F;

$BODYSTART = <<END;
<BODY LANG="" BGCOLOR="#C7C1A7" TEXT="#000000" LINK="#0000FF" VLINK="#800080" ALINK="#FF0000">
<FONT FACE="Verdana" Size="2">
END

# for the chtm file
$HBODYSTART = <<END;
<BODY>
<FONT FACE="Verdana" Size="2">
END

$BODYEND = <<END;
</FONT>
</BODY>
END

%TOC = ();
%TOCh = ();
%ITOC = ();
@TOCI = ();
%BODY = ();
$level = 0;
$chapter = 0;
$section = 0;
$subsection = 0;
$subsubsection = 0;
$bodindex = undef;

for $line (@lines){

# lines that are ignored 
  if( $line =~ /^\@title\s+(.*)$/ ){
    $TITLE = &htmlizee($1);
    next;
    }

  if( $line =~ /^\@author\s+(.*)$/ ){
    $AUTHOR = &htmlizee($1);
    next;
    }

  if( $line =~ /^\@chapter\s+(.*)$/ ){
    $chapter++;
    $section = 0;
    $subsection = 0;
    $subsubsection = 0;
    $bodindex = $chapter . '.';
    $TOC{$bodindex} = $1;
    $ITOC{$1} = $bodindex;
    push @TOCI,$bodindex;
    next;
    }

  if( $line =~ /^\@section\s+(.*)$/ ){
    $section++;
    $subsection = 0;
    $subsubsection = 0;
    $bodindex = $chapter . '.' . $section . '.' ;
    $TOC{$bodindex} = $1;
    $ITOC{$1} = $bodindex;
    push @TOCI,$bodindex;
    next;
    }

  if( $line =~ /^\@subsection\s+(.*)$/ ){
    $subsection++;
    $subsubsection = 0;
    $bodindex = $chapter . '.' . $section . '.' . $subsection . '.' ;
    $TOC{$bodindex} = $1;
    $ITOC{$1} = $bodindex;
    push @TOCI,$bodindex;
    next;
    }

  if( $line =~ /^\@subsubsection\s+(.*)$/ ){
    $subsubsection++;
    $bodindex = $chapter . '.' . $section . '.' . $subsection . '.' . $subsubsection . '.';
    $TOC{$bodindex} = $1;
    $ITOC{$1} = $bodindex;
    push @TOCI,$bodindex;
    next;
    }

  $BODY{$bodindex} .= $line;
  }

if( defined( $outfile ) ){
  $infile = $outfile;
  }else{
  $infile =~ s/\..*$//;
  }

open(RTF,">$infile.rtf") or die "Cannot open ${infile}.rtf";
print RTF <<END;
{\\rtf1\\ansi
{\\fonttbl
{\\f0\\fswiss\\fcharset0\\fprq2 Verdana;}
{\\f1\\fmodern\\fcharset0\\fprq1 Courier New;}}

{\\*\\generator t2h.pl;}
{\\info
{\\title $TITLE}
{\\author $AUTHOR}}

\\qc
{\\f0\\fs66 $TITLE}
\\ql
\\par
{\\f0\\fs18 by The VICE Team}
\\par\\par\\par\\par\\par
\\qc
{\\f0\\fs28 Table of Contents}
\\par
\\ql
\\fs18
END

open(FULL,">$infile.html") or die "Cannot open ${infile}.html";
print FULL <<END;
<HEAD>
<TITLE>$TITLE</TITLE>

<META NAME="description" CONTENT="$TITLE: Table of Contents">
<META NAME="keywords" CONTENT="$TITLE: Table of Contents">
<META NAME="resource-type" CONTENT="document">
<META NAME="distribution" CONTENT="global">
<META NAME="Generator" CONTENT="t2h.pl">

</HEAD>

$BODYSTART
<H1>$TITLE</H1>
<H3>by The VICE Team</H3>
<A NAME="contents"><H2>Table of Contents</H2></A>
<UL>
END


$outputfile = $infile . '_toc.html';
open(F,">$outputfile") or die "Can not open output file $outputfile";
print F <<END;
<HEAD>
<TITLE>$TITLE: Table of Contents</TITLE>

<META NAME="description" CONTENT="$TITLE: Table of Contents">
<META NAME="keywords" CONTENT="$TITLE: Table of Contents">
<META NAME="resource-type" CONTENT="document">
<META NAME="distribution" CONTENT="global">
<META NAME="Generator" CONTENT="t2h.pl">

</HEAD>

$BODYSTART
<H1>$TITLE</H1>
<H3>by The VICE Team</H3>
<H2>Table of Contents</H2>
<UL>
END
$plevel = 0;
for $toc ( @TOCI ){
  if( $toc =~ /\.1\.$/ ){
    print F "<UL>\n";
    print FULL "<UL>\n";
    }
  $level = $toc;
  $level =~ s/\d//g;
  $level = length($level);
  if( $level < $plevel ){
    print F "</UL>\n" x ($plevel - $level);
    print FULL "</UL>\n" x ($plevel - $level);
    }
  $plevel = $level;
  $tochtml = &htmlizee($TOC{$toc});
  print F <<END;
<A HREF="${infile}_${toc}html">$toc $tochtml</A><BR>
END
  print FULL <<END;
<A HREF="#$toc">$toc   $tochtml</A><BR>
END
  $tocrtf = &rtfizee($TOC{$toc});
  print RTF "\{\\f0\\fs",(18,18,16,12,12)[$level]," ","\\tab "x$level,"$toc $tocrtf\}\\par\n";
  }
print FULL "</UL>\n" x $plevel;
print RTF "\\page\n";
print F "</UL>\n" x $plevel;
print F <<END;
$BODYEND
</HTML>
END
close F;

#
# Create title page for the compiled help file
#
$outputfile = $infile . '_title.thtml';
open(F,">$outputfile") or die "Can not open output file $outputfile";
print F <<END;
<HEAD>
<TITLE>$TITLE</TITLE>

<META NAME="description" CONTENT="$TITLE">
<META NAME="keywords" CONTENT="$TITLE">
<META NAME="resource-type" CONTENT="document">
<META NAME="distribution" CONTENT="global">
<META NAME="Generator" CONTENT="t2h.pl">

</HEAD>

$HBODYSTART
<H1>$TITLE</H1>
<H3>by The VICE Team</H3>
</UL>
$BODYEND
</HTML>
END
close F;

#
# Create contents file for the compiled help file
#
$outputfile = $infile . '.hhc';
open(F,">$outputfile") or die "Can not open output file $outputfile";
print F <<END;
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML//EN">
<HTML>
<HEAD>
<meta name="GENERATOR" content="t2h.pl texi to html converter">
<!-- Sitemap 1.0 -->
</HEAD><BODY>
<OBJECT type="text/site properties">
	<param name="ImageType" value="Folder">
</OBJECT>
<UL>
END
$plevel = 0;
for $toc ( @TOCI ){
  if( $toc =~ /\.1\.$/ ){
    print F "<UL>\n";
    }
  $level = $toc;
  $level =~ s/\d//g;
  $level = length($level);
  if( $level < $plevel ){
    print F "</UL>\n" x ($plevel - $level);
    }
  $plevel = $level;
  $tochtml = &htmlizee($TOC{$toc});
  print F <<END;
<LI> <OBJECT type="text/sitemap">
<param name="Name" value="$tochtml">
<param name="Local" value="${infile}_${toc}thtml">
</OBJECT>
END
  }
print F <<END;
</UL>
$BODYEND
</HTML>
END
close F;

#
# Create project file for the compiled chtm file
#
$outputfile = $infile . '.hhp';
open(F,">$outputfile") or die "Can not open output file $outputfile";
print F <<END;
[OPTIONS]
Compatibility=1.1 or later
Compiled file=${infile}.chm
Contents file=${infile}.hhc
Default topic=${infile}_title.thtml
Display compile progress=No
Full-text search=Yes
Language=0x409 English (United States)
Title=$TITLE

[FILES]
${infile}_title.thtml
END
$plevel = 0;
for $toc ( @TOCI ){
  $level = $toc;
  $level =~ s/\d//g;
  $level = length($level);
  $plevel = $level;
  print F <<END;
${infile}_${toc}thtml
END
  }
print F <<END;
[INFOTYPES]
END
close F;

#
# Create each html and thtml files
#
for( $i=0 ; $i <= $#TOCI ; $i++ ){
  $toc = $TOCI[$i];
  $outputfile = $infile . '_' . $toc . 'html';
  open(F,">$outputfile") or die "Can not open output file $outputfile";
  $tochtml = &htmlizee($TOC{$toc});
print F <<END;
<HEAD>
<TITLE>$TITLE: $toc $tochtml</TITLE>

<META NAME="description" CONTENT="$TITLE: $tochtml">
<META NAME="keywords" CONTENT="$TITLE: $tochtml">
<META NAME="resource-type" CONTENT="document">
<META NAME="distribution" CONTENT="global">
<META NAME="Generator" CONTENT="t2h.pl">

</HEAD>

$BODYSTART
END

#
# Print the navigational links from this level upward
#
$ulc = 1;
print FULL '<P><a href="#contents">[Contents]</A><BR>' ,"\n";
print F '<UL><a href="' , $infile , '_toc.html">[Contents]</A><BR>' ,"\n";
if( $toc =~ /^(\d+\.)/ ){
  $section = $1;
  if( $toc ne $section ){
    $ulc ++;
    $tochtml = &htmlizee($TOC{$section});
    print F '<UL><a href="',$infile, '_' , $section , 'html">',
            $section,' ',$tochtml,"</A><BR>\n";
    }
  }

if( $toc =~ /^(\d+\.\d+\.)/ ){
  $section = $1;
  if( $toc ne $section ){
    $ulc ++;
    $tochtml = &htmlizee($TOC{$section});
    print F '<UL><a href="',$infile, '_' , $section , 'html">',
            $section,' ',$tochtml,"</A><BR>\n";
    }
  }

if( $toc =~ /^(\d+\.\d+\.\d+\.)/ ){
  $section = $1;
  if( $toc ne $section ){
    $ulc ++;
    $tochtml = &htmlizee($TOC{$section});
    print F '<UL><a href="',$infile, '_' , $section , 'html">',
            $section,' ',$tochtml,"</A><BR>\n";
    }
  }

if( $toc =~ /^(\d+\.\d+\.\d+\.\d+\.)/ ){
  $section = $1;
  if( $toc ne $section ){
    $ulc ++;
    $tochtml = &htmlizee($TOC{$section});
    print F '<UL><a href="',$infile, '_' , $section , 'html">',
            $section,' ',$tochtml,"</A><BR>\n";
    }
  }

while( $ulc-- ){ print F "</UL>\n"; }

print F "<P>\n";
print FULL "<P>\n";
print RTF "\\par\n";

$tochtml = &htmlizee($TOC{$toc});
print F "<H1>$toc $tochtml</H1>\n";
$dotcounter = $toc;
$n = $dotcounter =~ s/\.//g;
$tochtml = &htmlizee($TOC{$toc});
print FULL "<A name=\"$toc\"><H$n>$toc $tochtml</H$n></A>\n";
$tocrtf = &rtfizee($TOC{$toc});
print RTF "\\par\{\\f0\\fs",(4-$n)*2+18," \\b $toc $tocrtf\}\\par\n";
if( $i > 0 ){
  $STEPLINE = '<A HREF="' . $infile . '_' . $TOCI[$i-1] . 'html">[&lt;&lt;&lt;]</A> ';
  }else{
  $STEPLINE = '[&lt;&lt;&lt;] ';
  }

if( $i < $#TOCI ){
  $STEPLINE .= '<A HREF="' . $infile . '_' . $TOCI[$i+1] . 'html">[&gt;&gt;&gt;]</A>';
  }else{
  $STEPLINE .= '[&gt;&gt;&gt;]';
  }
print F "$STEPLINE\n";

#
# Print the navigational links from this level down
#

$ulc = 1;
$plevel = $toc;
$plevel =~ s/\d//g;
$plevel = length($plevel);
$toclen = $toc;
$toclen =~ s/\d//g;
$toclen=length($toclen);

for( $j = $i+1; $j <= $#TOCI ; $j++ ){

  $section = $TOCI[$j];
  $seclen = $TOCI[$j];
  $seclen =~ s/\d//g;
  $seclen = length($seclen);
  last if $toclen >= $seclen;

  $level = $section;
  $level =~ s/\d//g;
  $level = length($level);
  if( $level > $plevel ){
    print F "<UL>\n" x ($level - $plevel);
    $ulc += $plevel - $level;
    $ulc = 0 if $ulc < 0;
   print "Warning: daingling section \"$section $TOC{$section}\"\n" if $level > $plevel+1;
   }
  if( $level < $plevel ){
    print F "</UL>\n" x ($plevel - $level);
    $ulc -= $plevel - $level;
    $ulc = 0 if $ulc < 0;
    }
  $plevel = $level;
  $tochtml = &htmlizee($TOC{$section});
  print F <<END;
<A HREF="${infile}_${section}html">$section $tochtml</A><BR>
END
  }

while( $ulc-- ){ print F "</UL>\n"; }

print F "<HR>\n";
print F "<P>\n";

#
# print the body of that part
#

@lns = split /\n/ , $BODY{$toc};
@plns = ();
while( $#lns >= 0 ){
  $line = shift @lns;

  last if $line =~ /^\s*\@bye\s+/ ;
  last if $line =~ /^\s*\@bye$/ ;
  next if $line =~ /^\s*\@c\s+/;
  next if $line =~ /^\s*\@node\s+/;
  next if $line =~ /^\s*\@c$/;
  next if $line =~ /^\s*\@cindex\s+/;
  next if $line =~ /^\s*\@cindex$/;
  next if $line =~ /^\s*\@clindex\s+/;
  next if $line =~ /^\s*\@clindex$/;
  next if $line =~ /^\s*\@opindex\s+/;
  next if $line =~ /^\s*\@opindex$/;
  next if $line =~ /^\s*\@vindex\s+/;
  next if $line =~ /^\s*\@vindex$/;

  if( $line =~ /^\s*\@menu\s*$/ ){
    while( $#lns >= 0 && $line !~ /^\s*\@end\s+menu\s*$/ ){ $line = shift @lns; }
    next;
    }

  push @plns , $line;
  }
$line = &htmlizee( join( "\n", @plns));

print F  $line;
$line = &htmlizee( join( "\n", @plns),'#');
print FULL $line;
$line = &rtfizee( join( "\n", @plns));
print RTF $line;

  print F <<END;
<HR>
$STEPLINE
$BODYEND
</HTML>
END
close F;

#
# Create the thtml file
#
  $outputfile = $infile . '_' . $toc . 'thtml';
  open(F,">$outputfile") or die "Can not open output file $outputfile";

$tochtml = &htmlizee($TOC{$toc});
print F <<END;
<HEAD>
<TITLE>$TITLE: $tochtml</TITLE>

<META NAME="description" CONTENT="$TITLE: $tochtml">
<META NAME="keywords" CONTENT="$TITLE: $tochtml">
<META NAME="resource-type" CONTENT="document">
<META NAME="distribution" CONTENT="global">
<META NAME="Generator" CONTENT="t2h.pl">

</HEAD>

$HBODYSTART
END

print F "<H1>$tochtml</H1>\n";

if( $i > 0 ){
  $STEPLINE = '<A HREF="' . $infile . '_' . $TOCI[$i-1] . 'thtml">[&lt;&lt;&lt;]</A> ';
  }else{
  $STEPLINE = '[&lt;&lt;&lt;] ';
  }

if( $i < $#TOCI ){
  $STEPLINE .= '<A HREF="' . $infile . '_' . $TOCI[$i+1] . 'thtml">[&gt;&gt;&gt;]</A>';
  }else{
  $STEPLINE .= '[&gt;&gt;&gt;]';
  }
print F "$STEPLINE\n";

print F "<P>\n";
#
# print the body of that part
#

@lns = split /\n/ , $BODY{$toc};
@plns = ();
while( $#lns >= 0 ){
  $line = shift @lns;

  last if $line =~ /^\s*\@bye\s+/ ;
  last if $line =~ /^\s*\@bye$/ ;
  next if $line =~ /^\s*\@c\s+/;
  next if $line =~ /^\s*\@node\s+/;
  next if $line =~ /^\s*\@c$/;
  next if $line =~ /^\s*\@cindex\s+/;
  next if $line =~ /^\s*\@cindex$/;
  next if $line =~ /^\s*\@clindex\s+/;
  next if $line =~ /^\s*\@clindex$/;
  next if $line =~ /^\s*\@opindex\s+/;
  next if $line =~ /^\s*\@opindex$/;
  next if $line =~ /^\s*\@vindex\s+/;
  next if $line =~ /^\s*\@vindex$/;

  if( $line =~ /^\s*\@menu\s*$/ ){
    while( $#lns >= 0 && $line !~ /^\s*\@end\s+menu\s*$/ ){ $line = shift @lns; }
    next;
    }

  push @plns , $line;
  }
$line = &htmlizee( join( "\n", @plns) , 'thtml');

print F  $line;

  print F <<END;
<HR>
$STEPLINE
$BODYEND
</HTML>
END
close F;

  }
close FULL;
print RTF "\n\}\n";
close RTF;
