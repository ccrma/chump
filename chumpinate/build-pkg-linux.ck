@import "Chumpinate"

if (me.args() != 1) {
   <<< "ERROR: Please provide the version number" >>>;
   me.exit();
}

<<< "Generating Chumpinate package version " >>>;

// instantiate a Chumpinate package
Package pkg("Chumpinate");

// Add our metadata...
"Nick Shaheeed" => pkg.authors;

"https://github.com/ccrma/chump" => pkg.homepage;
"https://github.com/ccrma/chump" => pkg.repository;

"tbd" => pkg.license;
"Two classes (Package & PackageVersion) to help create packages to be used with ChuMP (the ChucK Manager of Packages)" => pkg.description;

["util", "chump", "packages"] => pkg.keywords;

// generate a package-definition.json
// This will be stored in "Chumpinate/package.json"
"./" => pkg.generatePackageDefinition;


<<< "Defining version " + me.arg(0) >>>;;

// Now we need to define a specific PackageVersion for test-pkg
PackageVersion ver("Chumpinate", me.arg(0));

"10.2" => ver.apiVersion;

"1.5.4.0" => ver.languageVersionMin;

"linux"=> ver.os;

ver.addFile("../builddir-release/chumpinate/Chumpinate.chug");

// wrap up all our files into a zip file, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "Chumpinate_linux", "https://ccrma.stanford.edu/~nshaheed/chugins/Chumpinate/0.1.0/linux/Chumpinate.zip");

// Generate a version definition json file, stores this in "chumpinate/<VerNo>/Chumpinate_linux.json"
ver.generateVersionDefinition("Chumpinate_linux", "./" );