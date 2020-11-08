#!/usr/bin/perl
# SPDX-FileCopyrightText: 2012-2016 Miika Turkia <miika.turkia@gmail.com>
#
# Try to locate RAW files to open them in external editor from
# GUI application like KPhotoAlbum

my @params;

# Raw extensions you use. If you use Ufraw you might want to add ufraw
# as the first extension in the list
# Using same file format list as KPhotoAlbum from
# http://www.cybercom.net/~dcoffin/dcraw/rawphoto.c
my @rawExt = (
	"3fr","arw","bay","bmq","cine","cr2","crw","cs1","dc2","dcr","dng","erf","fff","hdr","ia","k25","kc2","kdc","mdc","mef","mos","mrw","nef","nrw","orf","pef","pxn","qtk","raf","raw","rdc","rw2","sr2","srf","sti","tif","x3f","jpg"
);

# The application you use to develop the RAW files
my @raw_converters = ( "/usr/bin/AfterShot2X64", "/usr/bin/AfterShot2",
	"/usr/bin/AfterShotPro", "/usr/bin/bibble5",
	"/usr/bin/ufraw", "/usr/bin/rt", "/usr/bin/darktable" );
my @ASP_work = ( "/usr/bin/AfterShot3X64" );
my $extApp = "";
my $workApp = "";

foreach my $app (@ASP_work) {
	if ( -e $app ) {
		$workApp = $app;
		last;
	}
}

foreach my $app (@raw_converters) {
	if ( -e $app ) {
		$extApp = $app;
		last;
	}
}

# If you want to use specific converter, just assign it below
#$extApp = "/usr/bin/ufraw";

if ($extApp =~ m/^$/ && $workApp =~ m/^$/) {
	my $errMsg = "Could not find RAW developer. If you have one, " .
		"script open-raw.pl must be updated.";
	exec("notify-send \"$errMsg\"");
}

# A default regular expression for detecting the original RAW file
# We attempt to update this with the one used by KPhotoAlbum later
my $regexp = "(_(v){0,1}([0-9]){1,2}){0,1}\\.(jpg|JPG|tif|TIF|png|PNG)";

# Attempt to read the KPA's regular expression from configuration file
sub read_config {
	open CONFIG, "<", $ENV{"HOME"} . "/.kde/share/config/kphotoalbumrc" or return;
	while (<CONFIG>) {
		/modifiedFileComponent/ && do {
			$regexp = $_;
			$regexp =~ s/modifiedFileComponent=//;
			$regexp =~ s/\\\\/\\/g;
			chomp $regexp;
		};
	}
}

sub uniq {
	return keys %{{ map { $_ => 1 } @_ }};
}

read_config();

# Process the parameters and search for "original" files
foreach my $argnum (0..$#ARGV) {
	my $found = 0;
	my $file = "$ARGV[$argnum]";

	$file =~ s/$regexp//;

	foreach my $ext (@rawExt) {
		if (-e "$file.$ext") {
			push @params, "$file.$ext";
			$found = 1;
			last;
		} else {
			$ext = uc($ext);
			if (-e "$file.$ext") {
				push @params, "$file.$ext";
				$found = 1;
				last;
			}
		}
	}
	push @params, "$ARGV[$argnum]" if not $found;
}

my @uniqParams = uniq(@params);

if ($workApp =~ m/^.+$/) {
	my $workFile = "/tmp/kpa-asp-" . $$ . "_" . int(rand(100000)) . ".work";

	srand;
	open WORK, ">", $workFile;

	foreach my $file (@uniqParams) {
		print WORK $file . "\n";
	}

	close WORK;
	exec "$workApp $workFile";
	exit;
}

exec "$extApp @uniqParams";
