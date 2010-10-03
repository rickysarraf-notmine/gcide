#!/usr/local/bin/perl
# This file is -*- perl -*- source.
# Created: Wed Jan 4 13:04:37 1995 by faith@cs.unc.edu
# Revised: Mon Dec  9 15:22:26 1996 by faith@cs.unc.edu
# Public domain 1995 Rickard E. Faith (faith@cs.unc.edu)
#
# Special thanks to Nick Simicich (njs@scifi.gate.net) and Nicolai
# Langfeldt (janl@ifi.uio.no), who where very patient while answering all
# my stupid Perl questions.  Any errors and stylistic problems in this code
# are my fault, not theirs.
#

$oldfh = select(STDOUT); $| = 1; select($oldfh);

##@section = ( "section", "subsection", "subsubsection", "paragraph" );
##@Section = ( "Section", "Subsection", "Subsubsection", "Paragraph" );
@section = ( "subsection", "subsubsection", "paragraph" );
@Section = ( "Subsection", "Subsubsection", "Paragraph" );
$defaultCurrent = 0;
$current = $defaultCurrent;
$figextension = ".eepicemu";

sub bf {
    local($tmp) = $_[0];
    $tmp =~ s/_/\\_/g;
    $tmp =~ s/#/\\#/g;
    return "\\textbf{${tmp}}";
}

sub tt {
    local($tmp) = $_[0];
    $tmp =~ s/_/\\_/g;
    $tmp =~ s/#/\\#/g;
    return "\\emph{${tmp}}";
}

sub figure {
    local($tmp) = $_[0];
    "Figure~$tmp";
}

foreach $file (@ARGV) {
    printf STDERR "Reading ${file}\n";
    open (FILE, $file) || printf STDERR "Can't open $file: $!\n";
    @lines = (@lines,<FILE>);
}

@origlines = @lines;

$state = 0;
$endflag = 0;

foreach $line (@lines) {
    if (!$state && $line =~ s/^.*\\section\{(.*)\}/$1/) {
	$current = $defaultCurrent;
	&print_header($line);
	$state = 3;
	next;
    }
    if ((!$state || $state == 3) && $line =~ s/^.*\\subsection\{(.*)\}/$1/) {
	$current = $defaultCurrent + 1;
	printf STDERR "  * ";
	&print_header($line);
	$state = 3;
	next;
    }
    ++$current if ($line =~ s/\\bump\s//);
    --$current if ($line =~ s/\\unbump\s//);
    $state = 1 if ($line =~ s/\\doc\s//);
    $state = 3 if ($line =~ s/\\intro\s//);
    $state = 0 if ($line =~ s/\\endintro\s//);
    if ($state == 3) {
	local($result) = ($line =~ s/\*\///);

	$line =~ s/^\s*\*\s*//;
	push(@doc,$line);
	if ($result) {
	    &print_doc(@doc);
	    @doc = ();
	    &dump_secondary("\n");
	    $state = 0;
	}
    }
    if ($state == 1) {
 	$state = 2 if ($line =~ s/\*\///);
	push(@doc,$line);
    } elsif ($state == 2) {
	if ($endflag == 0 && $line !~ /^\s*{\s*$/) {
	    $endflag = 1 if ($line =~ /^.*{\s*$/);
	    $line =~ s/{//;
	    push(@decl,$line);
	} else {
	    &print_decl(@decl);
	    @decl = ();
	    &print_doc(@doc);
	    @doc = ();
	    &dump_secondary("\n");
	    $state = 0;
	    $endflag = 0;
	}
    }
}

&dump_secondary("\n");
    
sub print_header {
    $_[0] =~ s/^\s*//;
    $_[0] =~ s/\s*$//;
    $_[0] =~ s/\s*\*\///;
    printf STDERR "Processing \"${_[0]}\"\n";
    print "\n\\typeout{*** " . $Section[$current] . ": ${_[0]}}\n";
    print "\\" . $section[$current] . "{${_[0]}}\n\n";
}

sub print_decl {
    local($line);
    local($tmp);
    local($state) = 0;
    local(@input) = @_;

    foreach $line (@input) {
	if (!$state && $line =~ /\(/) {
	    $tmp = $line;
	    $tmp =~ s/^[^(]*[\s\*](\w+)\s*\(.*$/$1/;
	    chop $tmp;
	    printf STDERR  "  ${tmp}\n";
	    $tmp = &tt($tmp);
	    print "\n\\typeout{*** " . $Section[$current+1] . ": ${tmp}}\n";
	    print "\\" . $section[$current+1] . "{${tmp}}\n\n";
	    print "\\begin{lgrind}\n";
	    &lgrindopen;
	    $state = 1;
	}
	if ($state) {
	    print LGOUT $line;
	}
    }

    close(LGOUT);
    print while (<LGIN>);
    close(LGIN);
    wait;
    print "\\end{lgrind}\n\\vspace{2ex}\n\n\\noindent ";
}

sub lgrindopen {
    # This code is based on code from njs@scifi.gate.net (Nick Simicich)
    local($pid) = 0;
    close(LGIN);
#    close(LGOUT);
    pipe(LGIN, LGINCH) || die "Can't pipe LGIN: $!";
    pipe(LGOUTCH, LGOUT) || die "Can't pipe LGOUT: $!";
    # Unbuffer the output pipes.
    $oldfh = select(LGINCH); $| = 1; select($oldfh);
    $oldfh = select(LGOUT); $| = 1; select($oldfh);
    if($pid = fork) {       # parent
        close(LGINCH);
        close(LGOUTCH);
        return;
    }
    # Now in child
    close(LGOUT);
    open(STDIN, "<&LGOUTCH") || die "Can't dup STDIN: &!";
    close(LGOUTCH);
    
    close(LGIN);
    open(STDOUT, ">&LGINCH") || die "can't open STDOUT: &!";
    close(LGINCH);
    
    exec('lgrind','-i','-d','lgrindefs','-');
    die "exec must have failed, or I wouldn't be here: $!";
}

sub get_section {
    return $section[${_[0]}];
}

sub print_doc {
    local($line);

#    print "\n\\subsubsection{Description}\n\n";
    
    foreach $line (@_) {
	&dump_secondary($line);
	$line =~ s/\|([^|]*)\|/&tt($1)/ge;
	$line =~ s/\"([^"]*)\"/&bf($1)/ge; # "
	$line =~ s/\\val{([^{]*)}/&print_values($1)/ge;
	$line =~ s/\\fig{([^{]*)}{([^{]*)}/&do_figure($1,$2)/ge;
	$line =~ s/\\grind{([^{]*)}/&do_grind($1)/ge;
	$line =~ s/\\grindref{([^{]*)}/&figure("\\ref{$1}")/ge;
#        $line =~ s/\<([^>]*)\>/\\&get_section("$current+2"){$1}/ge;
	$line =~ s/^\s*\/\*\s*//;
	$line =~ s/^\s*//;
	$line =~ s/\s*$/\n/;
	print $line;
    }
}

sub print_values {
    local($target) = $_[0];
    local($line);
    local($state) = 0;
    local($result) = "";
    local($orig);

    foreach $orig (@lines) {
	$line = $orig;
	if ($state == 0) {
	    if ($line =~ /^\w+\s+${target}\s*\[\]\s*=\s*/) {
	        $state = 1;
		$line =~ s/^.*{//;
	    }
	}
        if ($state) {
	    if ($line =~ s/}.*$/\n/) {
	        $state = 2;
		$line =~ s/,\s*0\s*$/\n/;
            }
	    $line =~ s/^\s*//;
            $line =~ s/\s*$/ /;
	    $result .= $line;
	}
        if ($state == 2) {
	    $result =~ s/\s*$//;
	    return $result;
	}
    }
}

sub dump_secondary {
    local($line) = $_[0];

    if ($line =~ /^\s*\n$/ && $#secondary) {
	print @secondary;
	@secondary = ();
    }
}

sub do_grind {
    local($target) = $_[0];
    local($tt_target) = $target;
    local($_target) = $target;
    local($line);
    local($state) = 0;

    $tt_target = &tt($tt_target);
    $_target =~ s/_//g;

    push(@secondary,"\n\\begin{figure}[hbtp]\n\\begin{center}\n\\leavevmode");
    push(@secondary,"\n\\begin{lgrind}\n");
    &lgrindopen;

    foreach $line (@origlines) {
	next if !length($line);
	next if (!$state && $line =~ /^extern\s/);
	$state = 1 if (!$state && $line =~ /^[^(]*[\s\*]${target}\s*\(/);
	$state = 2 if (!$state && $line =~ /^\s*typedef\s.*\s${target}/);
	print LGOUT $line if ($state);
	last if ($state == 1 && $line =~ /^\s*}\s*\n$/);
        last if ($state == 2 && $line =~ /^\s*}.*;\s*$/);
    }

    close(LGOUT);
    push(@secondary,$_) while (<LGIN>);
    close(LGIN);
    wait;
    
    push(@secondary,"\\end{lgrind}\n");
    push(@secondary,"\n\\caption{${tt_target}}\n\\label{fig:${_target}}\n");
    push(@secondary,"\\end{center}\n\\end{figure}\n\n");

    return &figure("\\ref{fig:$_target}");
}

sub do_figure {
    local($target) = $_[0];
    local($caption) = $_[1];
    local($_target) = $target;
    local($line);
    local($state) = 0;

    $_target =~ s/_//g;
    $_target =~ s/\..*$//;

    push(@secondary,"\n\\begin{figure}[hbtp]\n\\begin{center}\n\\leavevmode");
    push(@secondary,"\\input{$target$figextension}\n");
    push(@secondary,"\n\\caption{$caption}\n\\label{fig:${_target}}\n");
    push(@secondary,"\\end{center}\n\\end{figure}\n\n");
    
    return &figure("\\ref{fig:$_target}");
}
