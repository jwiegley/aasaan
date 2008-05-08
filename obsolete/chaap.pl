#!/usr/bin/env perl

# chaap (print), version 1.1
#   by John Wiegley <johnw@gnu.org>
#
# Use "perl chaap --help" for help on script usage and options.
#
# This script takes an input file either of all original text
# paragraphs in Aasaan format, or of alternating original/translation
# paragraphs.  The rules governing the input format are as follows:
#
# 1. Alternate verses of the original language (encoded in *aasaan*
#    style), with translation.  Or, input only original verses.
#
# 2. At least one line of each original verse/paragraph must occur in
#    column one (that is, have no preceding whitespace).
#
# 3. Conversely, every line of the translation must begin with
#    whitespace.
#
# 4. The original verse and the translation should be separated by a
#    blank line.
#
# 5. Separate these verse/translation pairs with a blank line, except
#    when they occur in different paragraphs, when they should be
#    separated with two blank lines.
#
# 6. Intra-verse whitespace is allowed, for the sake of readability.
#
# 7. For poetic verse, precede the text with a greater-than sign,
#    followed by at least one space.
#
# 8. For Persian/Arabic poetry, if you wish each line to be stretched
#    to the same width, specific the width before the greater-than
#    character.

use Getopt::Long;
use aasaan;

my $translit 	 = 0;
my $latex 	 = 0;
my $arabic 	 = 0;
my $novocalize 	 = 0;
my $fullvocalize = 0;
my $original 	 = 0;
my $original_wp  = 0;
my $english 	 = 0;
my $help 	 = 0;

GetOptions ('translit'     => \$translit,
	    'latex'        => \$latex,
	    'arabic'       => \$arabic,
	    'novocalize'   => \$novocalize,
	    'fullvocalize' => \$fullvocalize,
	    'original'     => \$original,
	    'original-wp'  => \$original_wp,
	    'english'      => \$english,
	    'help'         => \$help);

if ($help) {
    print <<'EOB';
usage: chaap [options] <input text>

options:
  --original      output only original text; default is bilingual
  --original-wp   output original, with paragraph separation
  --english       output only english text (always with paragraphs)

  --arabic        use Arabic conventions, not Persian
  --latex         output code for LaTeX2e; default is HTML 4
  --novocalize    do not output vowellization marks
  --fullvocalize  output sukun, in addition to other diacriticals
  --translit      print transliterated text; default is to print
                  using Arabic script
EOB
    exit 0;
}

$vocal = "vocalize";
$vocal = "novocalize"   if $novocalize;
$vocal = "fullvocalize" if $fullvocalize;

$lang = "farsi";
$lang = "arab" if $arabic;

if ($latex) {
    print <<'EOB';
\documentclass[12pt]{article}

EOB
    if (1 or ! $translit) {
	print <<'EOB';
\usepackage{arabtex}
\usepackage{twoblks}
EOB
    }
    print <<'EOB';
\usepackage{geometry}
\geometry{verbose,
  paperwidth=8.5in,
  paperheight=11in,
  tmargin=1in,
  bmargin=1in,
  lmargin=1in,
  rmargin=1in,
  headheight=0in,
  headsep=0in,
  footskip=0in}

\pagestyle{plain}

\begin{document}
EOB
    if (! $translit) {
	print <<EOB;

\\setlength{\\parskip}{1\\baselineskip}

\\set$lang
\\$vocal
\\newarabfont

EOB
    }
} else {
    print <<'EOB';
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN">
<html>
  <head>
    <title></title>
    <meta name="generator" content="chaap">
    <meta http-equiv="Content-Type"
	  content="text/html;charset=utf-8">
  </head>
  <body>
    <table align="center" cellpadding="8">
EOB
}

my $bilingual = ! ($original or $original_wp or $english);
my $has_run = 0;

if ($latex) {
    if (! $bilingual) {
	if (! $english) {
	    print <<'EOB';
\begin{RLtext}
EOB
	}
    }
}

my $ruleset = ($translit ? ($latex ? \@aasaan::latex : \@aasaan::html) :
	       ($latex ? \@aasaan::arabtex : \@aasaan::unicode));
my $t_ruleset = $latex ? \@aasaan::latex : \@aasaan::html;

my $in_original = 0;
my $eng_buffer;
my $org_buffer;

my $arabic_mode = 0;

sub emit_verse {
    if ($in_original and ! ($bilingual or $english)) {
	$org_buffer = aasaan::xlat($ruleset, $org_buffer);
	if ($org_buffer =~ /\\rq(\s+\\\S+)*\s*\n/) {
	    $org_buffer .= " \\hspace{1ex}\n";
	}
	print $org_buffer;
    } elsif (! $in_original and ($bilingual or $english)) {
	if ($bilingual) {
	    $org_buffer = aasaan::xlat($ruleset, $org_buffer);
	    if ($latex) {
                $eng_buffer =~ s/\\\\$//o;
                $org_buffer =~ s/\\\\$//o;
		print <<EOB;

\\vspace{2ex}
\\twoblocks{
$eng_buffer}{
EOB
		print "\\begin{RLtext}\n" if ! $translit;
		print $org_buffer;
		print "\\end{RLtext}\n" if ! $translit;
		print "}\n";
	    } else {
		print <<EOB;

      <tr valign="top">
        <td width="50%">
          $eng_buffer        </td>
EOB
		if ($translit) {
		    print <<EOB;
        <td width="50%">
EOB
		} else {
		    print <<EOB;
        <td lang="ar" align="right" dir="RTL" width="50%">
EOB
		}
		print <<EOB;
          $org_buffer        </td>
      </tr>
EOB
	    }
	} else {
	    print $eng_buffer;
	}
    }

    if ($in_original) {
	$eng_buffer = "";
	if (! ($bilingual or $english)) {
	    $org_buffer = "";
	}
    } else {
	$org_buffer = "";
	#if (! $bilingual) {
	    $eng_buffer = "";
	#}
    }
    $in_original = 0;
}

my $was_verse = 0;

for (<ARGV>) {
    s/<<([^>]+)>>/aasaan::xlat($t_ruleset, $1)/geo;
    if ($latex) {
	s/<([^>]+)>/"<" . aasaan::xlat($ruleset, $1) . ">"/geo;
    }

    if (/^\s*$/o) {
	if ($eng_buffer or $org_buffer) {
	    emit_verse;

	    if ($was_verse) {
		if ($latex) {
		    $org_buffer .= "\n\n\\noindent ";
		} else {
		    $org_buffer .= "<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		}
	    }
	    $was_verse = 0;
	}
	elsif ($original_wp or $english) {
	    if ($latex) {
		print "\n\\noindent ";
	    } else {
		print "<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
	    }
	}
    } else {
	$in_original = 1 if ! $in_original and /^\S/;

	if ($in_original and /^[^>]*>/) {
	    if (! $was_verse and ! ($eng_buffer or $org_buffer)) {
		if ($latex) {
		    $org_buffer .= "\n\\noindent ";
		} else {
		    $org_buffer .= "<p>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
		}
	    }
	    $was_verse = 1;
	}

	if ($in_original) {
	    if (! $original_wp) {
		s/^([^>]+)?>( +)(.+)/$3/e;
	    } else {
		if ($latex) {
		    if (! $translit) {
			s/^([^>]+)>( +)(.+)/("\\hspace{1em}" x length $2) .
			    "\\spreadbox{" . $1 . "}{" . $3 . "} \\\\"/e;
		    }
		    s/^([^>]+)?>( +)(.+)/("\\hspace{1em}" x length $2) .
		      $3 . " \\\\"/e;
		} else {
		    s/^([^>]+)>( +)(.+)/("&nbsp;" x length $2) . $3 . "<br>"/e;
		}
	    }
	    $org_buffer .= $_;
	} else {
	    s/^    //;
	    if (! $original_wp) {
		s/^([^>]+)?>( +)(.+)/$3/e;
	    } else {
		if ($latex) {
		    s/^>( +)(.+)/("\\hspace{1em}" x length $1) . $2 . " \\\\"/e;
		} else {
		    if (/^>/) {
			$eng_buffer .= "<br>" unless $eng_buffer =~ /<br>$/;
		    }
		    s/^>( +)(.+)/("&nbsp;" x (10 * length $1)) . $2 . "<br>"/e;
		}
	    }
	    $eng_buffer .= $_;
	}
    }
}

emit_verse if $eng_buffer or $org_buffer;

if ($latex) {
    if (! $bilingual) {
	if (! $english) {
	    print <<'EOB';
\end{RLtext}
EOB
	}
    }
}

if ($latex) {
    print <<'EOB';

\end{document}
EOB
} else {
    print <<'EOB';
    </table>
  </body>
</html>
EOB
}
