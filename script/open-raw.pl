#!/usr/bin/perl
# Copyright 2012 Miika Turkia <miika.turkia@gmail.com>
#
# Try to locate RAW files to open them in external editor from
# GUI application like KPhotoAlbum

my @params;

# Raw extensions you use. If you use Ufraw you might want to add ufraw
# as the first extension in the list
# my @rawExt = ("ufraw", "CR2", "NEF");
my @rawExt = ("CR2", "NEF");

# The application you use to develop the RAW files
# my $extApp = "/usr/bin/ufraw";
my $extApp = "/usr/bin/AfterShotPro";

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
		}
	}
	push @params, "$ARGV[$argnum]" if not $found;
}

my @uniqParams = uniq(@params);
exec "$extApp @uniqParams";
