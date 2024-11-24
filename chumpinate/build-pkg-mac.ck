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

"mac"=> ver.os;

ver.addFile("../builddir-release/chumpinate/Chumpinate.chug");

"chugins/Chumpinate/" + ver.version() + "/" + ver.os() + "/Chumpinate.zip" => string path;

<<< path >>>;

// wrap up all our files into a zip file, and tell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "Chumpinate_mac", "https://ccrma.stanford.edu/~nshaheed/" + path);

chout <= "Use the following commands to upload the package to CCRMA's servers:" <= IO.newline();
chout <= "ssh nshaheed@ccrma-gate.stanford.edu \"mkdir -p ~/Library/Web/chugins/Chumpinate/"
      <= ver.version() <= "/" <= ver.os() <= "\"" <= IO.newline();
chout <= "scp Chumpinate_mac.zip nshaheed@ccrma-gate.stanford.edu:~/Library/Web/" <= path <= IO.newline();

// Generate a version definition json file, stores this in "chumpinate/<VerNo>/Chumpinate_mac.json"
ver.generateVersionDefinition("Chumpinate_mac", "./" );