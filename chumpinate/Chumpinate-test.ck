@import "Chumpinate"
@import "HashMap"

// instantiate a Chumpinate package
Package pkg("test-pkg");

// Add our metadata...
"nick" => pkg.authors;

["nick", "ge"] => pkg.authors;

"www.www.com" => pkg.homepage;
"www.www.com" => pkg.repository;

"mit" => pkg.license;
"a description" => pkg.description;

["wow", "much", "keyword"] => pkg.keywords;

// generate a package-definition.json
// This will be stored in "test-pkg/package.json"
"./" => pkg.generatePackageDefinition;


// Now we need to define a specific PackageVersion for test-pkg
PackageVersion ver("test-pkg", "1.0.0");

"10.2" => ver.apiVersion;

"1.5.4.2" => ver.languageVersionMin;

"1.5.4.3" => ver.languageVersionMax;

"linux" => ver.os;

"x86_64" => ver.arch;

// Add a bunch of files that are found in this version of the pkg
ver.addFile("./test.txt");
ver.addDataFile("./test.txt");
ver.addExampleFile("./test.txt", "test");
ver.addDocsFile("./test.txt");
ver.addDepsFile("./test.txt");

// Add a dependency
ver.addPackageDependency("Rec", "1.0.0", "2.0.0");

// wrap up all our files into a zip file, and tcell Chumpinate what URL
// this zip file will be located at.
ver.generateVersion("./", "test-pkg_linux", "ccrma.stanford.edu/~nshaheed/test-pkg.zip");

// Generate a version definition json file, stores this in "test-pkg/1.0.0/test-pkg_linux.json"
ver.generateVersionDefinition("test-pkg_linux", "./");

// assert that version defn is what we want

// Test Harness
public class T
{
    fun static void println(string s) {
        chout <= s <= IO.nl();
    }

    fun static void err(string s) {
        cherr <= s <= IO.nl();
    }

    fun static void assert(int bool) {
        if (!bool) err("Assertion failed");
    }

    fun static void assert(int bool, string s) {
        if (!bool) err("Assertion failed: " + s);
    }

    fun static int feq(float a, float b) {
        return Math.fabs(a - b) < 0.0001;
    }

    fun static print(string a[]) {
        for (string s : a) {
            chout <= s <= " ";
        }
        chout <= IO.nl();
    }
}

HashMap.fromJsonFile(me.dir() + "test-pkg/1.0.0/test-pkg_linux.json") @=> HashMap h;

T.assert(h.get("api_version").getInt("major") == 10, "wrong api major version");
T.assert(h.get("api_version").getInt("minor") == 2, "wrong api minor version");

T.assert(h.getStr("arch") == "x86_64", "wrong arch");

T.assert(h.get("dependencies").size() == 1, "should only have 1 dependency");

T.assert(h.get("dependencies").get(0).getStr("name") == "Rec");
T.assert(h.get("dependencies").get(0).getStr("version_min") == "1.0.0");
T.assert(h.get("dependencies").get(0).getStr("version_max") == "2.0.0");

T.assert(h.get("files").size() == 1, "should be 1 file");
T.assert(h.get("files").get(0).has("checksum"), "should have checksum field");
T.assert(h.get("files").get(0).getStr("file_type") == "zip", "should be zip file");
T.assert(h.get("files").get(0).getStr("local_dir") == "./", "local dir should be top-level");
T.assert(h.get("files").get(0).getStr("url") == "ccrma.stanford.edu/~nshaheed/test-pkg.zip", "wrong url");

T.assert(h.get("language_version_min").getInt("mega") == 1, "wrong min ck version (mega)");
T.assert(h.get("language_version_min").getInt("major") == 5, "wrong min ck version (major)");
T.assert(h.get("language_version_min").getInt("minor") == 4, "wrong min ck version (minor)");
T.assert(h.get("language_version_min").getInt("patch") == 2, "wrong min ck version (patch)");

T.assert(h.get("language_version_max").getInt("mega") == 1, "wrong max ck version (mega)");
T.assert(h.get("language_version_max").getInt("major") == 5, "wrong max ck version (major)");
T.assert(h.get("language_version_max").getInt("minor") == 4, "wrong max ck version (minor)");
T.assert(h.get("language_version_max").getInt("patch") == 3, "wrong max ck version (patch)");

<<< "SUCCESS" >>>;