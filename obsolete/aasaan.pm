package aasaan;

$VERSION = '1.00';		# written by John Wiegley <johnw@gnu.org>

# Persian Transliteration conventions, "aasaan" style.
#
# These rules are derived from many sources, based on attempts to find
# a readable, easily edited source format that is unambiguous in all
# circumstances and sufficient to faithfully reproduce the original
# text with or without full vowellization (`iraab).  Example:
#
#   va shar.t ast iin `ibaad-raa kih dil-raa kih manba`-i khaziinih-i
#   il_ahiyyih ast az har naqsh-ii paak kunand va az taqliid kih az
#   athar-i aabaa' va ajdaad ast i`raa.d namaayand va abvaab-i duustii
#   va dushmanii-raa baa kull-i ahl-i ar.d masduud kunand
#
# This Perl module supports conversion of this transliteration format
# into:
#
#   Unicode 2.0 coding
#   ArabTeX input coding
#   LaTeX coding for Baha'i "House Style"
#   HTML 4.0 coding for Baha'i "House Style"
#
# To use this module, simply import it and call aasaan::xlat, passing
# the desired set of output generation rules, and the string to be
# transformed.  The converted string is returned.  Example:
#
#   #!/usr/bin/env perl -w
#   use aasaan;
#   for (<ARGV>) {
#     print aasaan::xlat(\@aasaan::html, $_);
#   }

# "Aasaan" style conventions:
#
# Most consonants are written as expected.  Letter pairs underlined in
# Baha'i House Style are written: sh ch kh th gh dh zh.  To split into
# two separate letters, separate with _.  Letters printed with a dot
# below in House Style are written with a period: .h .s .d .t .z.
#
# For tashdid, double the desired letter: mubashshir.
# Hamza and madda are generated from context in most cases.
#
# Vowels:                   a, i, u, aa, ii, uu, ey, uw
# Defective alef:           _a
# Explicit hamzih:          '
# Intermediate silent vav:  w, as in khwastan
# Final silent vav:         o, as in to
# Non-silent hih (do chashm): H
# Tanween:                  N
# Tanween (explicit alif):  NA
# Tah marbuta:              T
# Alif maqsura:             Y
# Izafe:                    -i
# Non-specific suffix:      -ii
# Prefix bi:                bi-
# Prefix mii:               mii, mii- to prevent joining
# Suffix raa:               -raa
# Plural suffixes:          -haa -aan
# Short copula verbs:       -am -ii -ast -iim -iid -and
# Possession suffixes:      -at -ash
# Quotes:                   `` and ''. For tashdiid on `eyn use: ~`
#
# Put ^ before a word to capitalize it in transliteration output.

my @sortorder = (
    'a/aa/Y/_a', 'b', 'p', 't/T', 'th', 'j', 'ch', '.h', 'kh', 'd',
    'dh', 's', 'sh', 'r', 'z', 'zh', '.s', '.d', '.t', '.z', '`',
    'gh', 'f', 'q', 'k', 'g', 'l', 'm', 'n/N', 'v/uu/uw/o', 'y/ii/ey',
    'H/h',
);
my %lookup = ();

my $index = 1;
for $letter (@sortorder) {
    for $elem (split "/", $letter) {
	$lookup{$elem} = $index;
    }
    $index++;
}

@latex = (
    [ '\\\\set(arab|farsi)', '' ],

    [ '\\^([sctdkgz])_h', '\u$2h' ],
    [ '([sctdkgz])_h', '$2h' ],

    [ 'aa-ii(\\s+)', '\\\\\'{a}\'\\\\\'{\\\\i}$2' ],
    [ 'uu-ii(\\s+)', '\\\\\'{u}\'\\\\\'{\\\\i}$2' ],
    [ 'ii-i\\s+', '\\\\\'{\\\\i}y-i-' ],	# yey ba hamza

    [ '\\^aa', '\\\\\'{A}' ],
    [ '\\^uu', '\\\\\'{U}' ],
    [ '\\^ii', '\\\\\'{I}' ],

    [ 'waa', '\\\\\'{a}' ],	# silent vav
    [ 'aa', '\\\\\'{a}' ],
    [ '_a', '\\\\\'{a}' ],	# defective alif
    [ 'uu', '\\\\\'{u}' ],
    [ 'wuu', '\\\\\'{u}' ],	# silent vav
    [ 'ii', '\\\\\'{\\\\i}' ],
    [ 'wii', '\\\\\'{\\\\i}' ],	# silent vav

    [ '\\^ey', 'Ay' ],		# diphthong
    [ 'ey', 'ay' ],		# diphthong
    [ 'uw', 'aw' ],		# diphthong
    [ 'o', 'uw' ],		# final silent vav

    [ '\\^\\.h', '\\\\d{H}' ],
    [ '\\^\\.s', '\\\\d{S}' ],
    [ '\\^\\.d', '\\\\d{D}' ],
    [ '\\^\\.t', '\\\\d{T}' ],
    [ '\\^\\.z', '\\\\d{Z}' ],

    [ '\\.h', '\\\\d{h}' ],
    [ '\\.s', '\\\\d{s}' ],
    [ '\\.d', '\\\\d{d}' ],
    [ '\\.t', '\\\\d{t}' ],
    [ '\\.z', '\\\\d{z}' ],

    [ 'h-i\\s+', 'y-i-' ],	# izafe ba hamza
    [ '-i\\s+', '-i-' ],		# izafe
    [ 'H', 'h' ],		# non-silent heh
    [ 'NA?', 'n' ],		# tanween
    [ 'T', 't' ],		# tah marbuta
    [ 'Y', '\\\\\'{a}' ],	# alif maqsura

    [ '\\^sh', '\\\\underline{Sh}' ],
    [ '\\^ch', '\\\\underline{Ch}' ],
    [ '\\^th', '\\\\underline{Th}' ],
    [ '\\^dh', '\\\\underline{Dh}' ],
    [ '\\^kh', '\\\\underline{Kh}' ],
    [ '\\^gh', '\\\\underline{Gh}' ],
    [ '\\^zh', '\\\\underline{Zh}' ],

    [ 'sh', '\\\\underline{sh}' ],
    [ 'ch', '\\\\underline{ch}' ],
    [ 'th', '\\\\underline{th}' ],
    [ 'dh', '\\\\underline{dh}' ],
    [ 'kh', '\\\\underline{kh}' ],
    [ 'gh', '\\\\underline{gh}' ],
    [ 'zh', '\\\\underline{zh}' ],

    [ '\\^mii-', 'M\\\\\'{\\\\i}' ],
    [ 'mii-', 'm\\\\\'{\\\\i}' ],

    [ '\\^(`?.)', '\u$2' ],

    [ '([AP]/|/[AP])', '' ],

    [ '_', '' ],
    [ '~`', '`' ],
    [ '"', '' ],
    [ '\\|', '' ],
);

@arabtex = (
    [ '\\^', '' ],
    [ '([sctdkgz])_h', '$2h' ],

    [ '(?<=[^aiu][aiu])h-(?=ii)', 'H\\\\hspace{0.4ex}' ],
    [ '(?<=[^aiu][aiu])h-(?=[^i])', 'H\\\\hspace{0.4ex}' ],
    [ '(?<=[^aiu][aiu])h-(?=i)', 'H-' ],
    [ '(?<=(aa|uu|ii))-(?=a(sh|t|m))', '-y' ],
    [ 'H-', 'h-' ],
    [ "'-(?=(ii?|I)\b)", "'" ],

    [ 'v', 'w' ],

    [ 'aa', 'A' ],
    [ 'uu', 'U' ],
    [ 'ii-i\b', 'I' ],
    [ 'ii', 'I' ],

    [ 'Y', 'Y_a' ],
    [ 'ey', 'ey' ],
    [ 'uw', 'ow' ],
    [ '\bo\b', 'w' ],		# va pronounced as o
    [ 'o\b', 'ow' ],		# final silent vaav

    [ '\.s', '.s' ],
    [ '\.d', '.d' ],
    [ '\.t', '.t' ],
    [ '\.z', '.z' ],

    [ 'sh', '^s' ],
    [ 'ch', '^c' ],
    [ 'th', '_t' ],
    [ 'dh', '_d' ],
    [ 'kh', '_h' ],
    [ 'gh', '.g' ],
    [ 'zh', '^z' ],

    #[ '[aiu]N\b', '\"$1' ],

    # prefixes and suffixes

    [ '-raa\b', '\\\\hspace{0.4ex}rA' ],
    [ '\bmii-', 'mI\\\\hspace{0.4ex}' ],
    [ '\bbar-', 'bar\\\\hspace{0.4ex}' ],
    [ '\bbi-(?=[^aiu])', 'bi|' ],

    [ '([l])-a(?=(sh|t))', '$2|a' ],

    # special cases

    [ '~`', '``' ],
    [ '_a', '_a' ],
    [ '_', '|' ],

    # quotations

    [ '``', '\\\\lq ' ],
    [ "''", '\\\\rq ' ],

    # language changes

    [ 'A/', '\\\\setarab \\\\newtanwin \\\\fullvocalize' ],
    [ '/A', '\\\\setfarsi \\\\newtanwin \\\\vocalize' ],
    [ 'P/', '\\\\setfarsi \\\\newtanwin \\\\vocalize' ],
    [ '/P', '\\\\setarab \\\\newtanwin \\\\fullvocalize' ],
);

@html = (
    [ '``', '\\"' ],
    [ "''", '\\"' ],
    [ '"', '' ],

    [ '\\\\set(arab|farsi)', '' ],

    [ '\\^([sctdkgz])_h', '\u$2h' ],
    [ '([sctdkgz])_h', '$2h' ],

    [ 'aa-ii(\\s+)', "&aacute;'&iacute;\$2" ],
    [ 'uu-ii(\\s+)', "&uacute;'&iacute;\$2" ],
    [ 'ii-i\\s+', '&iacute;y-i-' ],	# yey ba hamza

    [ '\\^aa', '&Aacute;' ],
    [ '\\^uu', '&Uacute;' ],
    [ '\\^ii', '&Iacute;' ],

    [ 'waa', '&aacute;' ],	# silent vav
    [ 'aa', '&aacute;' ],
    [ '_a',  '&aacute;' ],
    [ 'uu', '&uacute;' ],
    [ 'wuu', '&uacute;' ],
    [ 'ii', '&iacute;' ],
    [ 'wii', '&iacute;' ],

    [ '\\^ey', 'Ay' ],
    [ 'ey', 'ay' ],
    [ 'uw', 'aw' ],
    [ 'o', 'uw' ],		# final silent vav

    [ '\\^\\.h', '&#7716;' ],
    [ '\\^\\.s', '&#7778;' ],
    [ '\\^\\.d', '&#7692;' ],
    [ '\\^\\.t', '&#7788;' ],
    [ '\\^\\.z', '&#7826;' ],

    [ '\\.h', '&#7717;' ],
    [ '\\.s', '&#7779' ],
    [ '\\.d', '&#7693;' ],
    [ '\\.t', '&#7789;' ],
    [ '\\.z', '&#7827;' ],

    [ 'h-i\\s+', 'y-i-' ],	# izafe ba hamza
    [ '-i\\s+', '-i-' ],	# izafe
    [ 'iyyih\b', '&iacute;yyih' ],	# izafe

    [ '([aiu])\\s+a([^-]+)-$2', "$1'$2-$2" ],

    [ 'H',   'h' ],
    [ 'NA?', 'n' ],
    [ 'T',   't' ],
    [ 'Y',   '&aacute;' ],

    [ '\\^sh', '<u>Sh</u>' ],
    [ '\\^ch', '<u>Ch</u>' ],
    [ '\\^th', '<u>Th</u>' ],
    [ '\\^dh', '<u>Dh</u>' ],
    [ '\\^kh', '<u>Kh</u>' ],
    [ '\\^gh', '<u>Gh</u>' ],
    [ '\\^zh', '<u>Zh</u>' ],

    [ 'sh', '<u>sh</u>' ],
    [ 'ch', '<u>ch</u>' ],
    [ 'th', '<u>th</u>' ],
    [ 'dh', '<u>dh</u>' ],
    [ 'kh', '<u>kh</u>' ],
    [ 'gh', '<u>gh</u>' ],
    [ 'zh', '<u>zh</u>' ],

    [ '\\^mii-', 'M&iacute;' ],
    [ 'mii-', 'm&iacute;' ],

    [ '\\^(`?.)', '\U$2' ],

    [ '([AP]/|/[AP])', '' ],

    [ '_', '' ],
    [ '~`', '`' ],
    [ '"', '' ],
    [ '\\|', '-' ],
);

@unicode = (
    [ '\\^', '' ],
    [ '``', '\\"' ],
    [ "''", '\\"' ],
    [ '"', '' ],

    [ '\\\\set(arab|farsi)', '' ],

    [ '(\\.?[bptjdsrzfqkglmnvyh]|[sctdkgz]h)(?=\\2)', '&#1617;' ],

    [ '~`', '&#1617;&#1593;' ],
    [ 'aNA?', '&#1575;&#1611;' ],
    [ 'uNA?', '&#1612;' ],
    [ 'iNA?', '&#1575;&#1613;' ],
    [ 'aa', '&#1575;' ],
    [ 'aa-ii\\b', '&#1575;&#1569;&#1609;' ],
    [ 'aa', '&#1575;' ],
    [ 'Y', '&#1575;' ],
    [ '_a', '' ],
    [ '\\baa', '&#1570;' ],
    [ '`ii-i(?=\\s+)', '&#1593;&1574;' ], # yey ba hamza
    [ '`ii\\b', '&#1593;&#1609;' ],
    [ '`ii', '&#1593;&#1610;' ],
    [ 'ii-i(?=\\s+)', '&1574;' ], # yey ba hamza
    [ '\\buu', '&#1575;&#1608;' ],
    [ 'b', '&#1576;' ],
    [ 'p', '&#1662;' ],
    [ 'th', '&#1579;' ],
    [ 't', '&#1578;' ],
    [ 'j', '&#1580;' ],
    [ 'ch', '&#1670;' ],
    [ '\\.h', '&#1581;' ],
    [ 'kh', '&#1582;' ],
    [ 'dh', '&#1584;' ],
    [ 'd', '&#1583;' ],
    [ 'sh', '&#1588;' ],
    [ 's', '&#1587;' ],
    [ 'r', '&#1585;' ],
    [ 'zh', '&#1688;' ],
    [ 'z', '&#1586;' ],
    [ '\\.s', '&#1589;' ],
    [ '\\.d', '&#1590;' ],
    [ '\\.t', '&#1591;' ],
    [ '\\.z', '&#1592;' ],
    [ 'gh', '&#1594;' ],
    [ 'f', '&#1601;' ],
    [ 'q', '&#1602;' ],
    [ 'k', '&#1705;' ],
    [ 'g', '&#1711;' ],
    [ 'l', '&#1604;' ],
    [ 'm', '&#1605;' ],
    [ 'n', '&#1606;' ],
    [ 'o', '&#1608;' ],
    [ 'v', '&#1608;' ],
    [ 'w', '&#1608;' ],
    [ 'uw', '&#1608;' ],
    [ 'uu', '&#1608;' ],
    [ 'ey\\b', '&#1609;' ],
    [ 'ey', '&#1610;' ],
    [ 'ii\\b', '&#1609;' ],
    [ '\\bii', '&#1575;&#1610;' ],
    [ 'ii', '&#1610;' ],
    [ 'y\\b', '&#1609;' ],
    [ 'y', '&#1610;' ],
    [ 'h', '&#1607;' ],
    [ 'T', '&#1577;' ],

    # jww (2002-08-20): fix davaan-and
    [ 'h-(?=(a(m|s[th]|nd)|ii(d|m)?)\\s+)', '&#1607; ' ],
    [ 'h-i(?=\\s+)', '&#1607;&#1569;' ], # izafe ba hamza
    [ 'H', '&#1607;' ],

    [ '-i(\\s+)', '$2' ],
    [ '-(?=raa)', ' ' ],
    [ '-', '' ],
    [ '`[aui]', '&#1593;' ],
    [ '`', '&#1593;' ],
    [ '\\b[aui]', '&#1575;' ],
    [ '[aui]', '' ],

    [ '([AP]/|/[AP])', '' ],

    [ '\\|', '' ],
    [ '_', '' ],

    [ '0', '&#1776;' ],
    [ '1', '&#1777;' ],
    [ '2', '&#1779;' ],
    [ '3', '&#1780;' ],
    [ '4', '&#1781;' ],
    [ '5', '&#1782;' ],
    [ '6', '&#1783;' ],
    [ '7', '&#1784;' ],
    [ '8', '&#1785;' ],
    [ '9', '&#1786;' ],
);

sub xlat ($$) {
    my ($style, $str) = @_;

    $result = "";
  LOOP:
    for ($i = 0, $l = length ($str); $i < $l; ) {
	for $rule (@$style) {
	    pos $str = $i;
	    if ($str =~ /\G($rule->[0])/) {
		$result .= eval '"' . $rule->[1] . '"';
		$i += length $1;
		next LOOP;
	    }
	}
	$result .= substr $str, $i++, 1;
    }
    return $result;
}

sub split_letters {
    my ($word, $keep_initial) = @_;
    my (@letters, $i, $l, $let, $found, $pre_i);

    for ($i = 0, $l = length $word; $i < $l; ) {
	$pre_i = $i;
	$found = 0;

	if ($i + 2 <= $l) {
	    $let = substr $word, $i, 2;
	    if (exists $lookup{$let}) {
		$i += 2;
		$found = 1;
	    }
	}

	$let = substr $word, $i++, 1 unless $found;
	next if ((! $keep_initial or $pre_i > 0) and
		 ($let eq "a" or $let eq "i" or $let eq "u"));

	push @letters, $let;
    }
    return @letters;
}

my @correspondences = (
    [ '.d', 'z', '.z', 'dh' ],
    [ '.h', 'h', 'H' ],
    [ '.s', 's', 'th' ],
    [ '.t', 't', 'T' ],
    [ 'N', 'n' ],
    [ 'Y', 'aa', '_a' ],
    [ '`', "'" ],
    [ 'ey', 'ii', 'y' ],
    [ 'gh', 'q' ],
    [ 'n', 'm' ],
    [ 'o', 'uu', 'uw', 'wa', 'v' ],
);

sub generate_strings ($$$) {
    my ($letters, $sofar, $phonetic) = @_;
    my $isarray = 0;

    return @$sofar if ! @$letters;

    my $let = shift @$letters;
    if ($phonetic or $let eq "y" or $let eq "v") {
      OUTER:
	for $corr (@correspondences) {
	    for $memb (@$corr) {
		$let = $corr, $isarray = 1, last OUTER
		    if $memb eq $let;
	    }
	}
    }
    $let = [ $let ] if ! $isarray;

    my @lets = ();
    if (@$sofar) {
	for $l (@$let) {
	    push(@lets, $l), next
		if ($l eq "aa" or $l eq "ii" or $l eq "uu" or
		    $sofar->[0] =~ /[aiu]$/o);

	    push @lets, $l;
	    push @lets, "a" . $l;
	    push @lets, "i" . $l;
	    push @lets, "u" . $l;

	    # If generating possible phonetic matches, make sure to
	    # check for tashdid
	    if ($phonetic) {
		push @lets, $l . $l;
		push @lets, "a" . $l . $l;
		push @lets, "i" . $l . $l;
		push @lets, "u" . $l . $l;
	    }
	}
    } else {
	@lets = @$let;
    }

    my @bucket = ();
    if (! @$sofar) {
	@bucket = @lets;
    } else {
	for $memb (@$sofar) {
	    for $l (@lets) {
		push @bucket, $memb . $l;
	    }
	}
    }
    return generate_strings($letters, \@bucket, $phonetic);
}

sub similar {
    my ($word, $phonetic) = @_;
    return generate_strings [ split_letters($word, 1) ], [], $phonetic;
}

sub compare ($$) {
    my ($left, $right) = @_;

    my (@l, $l, $ll);
    my (@r, $r, $lr);

    for (@l = split_letters $left,  $l = 0, $ll = length @l,
	 @r = split_letters $right, $r = 0, $lr = length @r;
	 $l < $ll and $r < $lr; $l++, $r++) {
	my $diff = $lookup{$l[$l]} - $lookup{$r[$r]};
	return $diff if $diff != 0;
    }

    return -1 if $l < $ll;
    return  1 if $r < $lr;
    return  0;
}

1;
