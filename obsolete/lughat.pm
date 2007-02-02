package lughat;

$VERSION = '1.00';	     # written by John Wiegley <johnw@gnu.org>

use aasaan;

%index = ();

sub open ($) {
    my ($base) = @_;

    if (-r "$base.idx" and ((stat(_))[9] > (stat("$base.txt"))[9])) {
	open (INDEX, "$base.idx") or die;
	while (<INDEX>) {
	    chomp;

	    my @elems = split /:/;
	    my $term = $elems[0];
	    shift @elems;

	    $index{$term} = [];
	    for $elem (@elems) {
		push @{$index{$term}}, [ split /\//, $elem ];
	    }
	}
	close INDEX;
    } else {
	my $pos = 0, $page = 1;
	open (DICT, "$base.txt") or
	    die "Could not find \"$base.txt\"!";
	while (<DICT>) {
	    $page = $1 if s/\s*{P([0-9]+)}//o;
	  TERM:
	    for $term (/<([^>]+)>/g) {
		next if ! $term or length $term < 3;

		if (exists $index{$term}) {
		    for $i (@{$index{$term}}) {
			next TERM if abs $i->[0] == abs $pos;
		    }
		    push @{$index{$term}}, [ $pos, $page ];
		} else {
		    $index{$term} = [ [ $pos, $page ] ];
		}

	      SUBTERM:
		for $subterm (split /[- ]/, $term) {
		    if (exists $index{$subterm}) {
			for $i (@{$index{$subterm}}) {
			    next SUBTERM if abs $i->[0] == abs $pos;
			}
			push @{$index{$subterm}}, [ -$pos, $page ];
		    } else {
			$index{$subterm} = [ [ -$pos, $page ] ];
		    }
		}
	    }
	    $pos = tell DICT;
	}
	close DICT;

	open (INDEX, "> $base.idx") or die;
	for $term (sort keys %index) {
	    print INDEX $term;
	    for $loc (@{$index{$term}}) {
		print INDEX ":", $loc->[0], "/", $loc->[1];
	    }
	    print INDEX "\n";
	}
	close INDEX;
    }
}

sub close ($) {
    %index = ();
}

sub read ($$) {
    my ($base, $pos) = @_;
    open (DICT, "$base.txt") or
	die "Could not find \"$base.txt\"!";
    seek DICT, $pos, 0;
    my $def = <DICT>;
    close DICT;
    chomp $def;
    $def =~ s/\s*{P([0-9]+)}//o;
    return $def;
}

sub read_page ($$) {
    my ($base, $page) = @_;
    open (DICT, "$base.txt") or
	die "Could not find \"$base.txt\"!";
    my $text = "";
    my $nextpage = $page + 1;
    while (<DICT>) {
	$text .= $_ if /{P$page}/ .. /{P$nextpage}/;
	last if /{P$nextpage}/;
    }
    chomp $text;
    $text =~ s/\s*{P([0-9]+)}//og;
    return $text;
}

1;
