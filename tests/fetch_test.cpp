#include "fetch.h"
#include "package.h"
#include <catch2/catch_test_macros.hpp>

// TEST_CASE("Fetch a package", "[Fetch]") {
//   Fetch f;

//   std::string path = "./data/test-data.json";

//   Package want = {"John", "PackageA", "1.0", "9.0", "http://example.com/butt.chug", "http://example.com", "http://repo.com", "specA", {"AuthorA", "AuthorB"}, "MIT", "DescriptionA", {"KeywordA", "KeywordB"}};  
  
//   auto got = f.fetch(path);

//   // std::cout << want << std::endl << std::endl << std::endl << std::endl;

//   REQUIRE(got == want);
// }

// TEST_CASE("Check if a filepath is a valid JSON file", "[Fetch]") {
//   Fetch f;

//   CHECK(f.isJSONFile("data.json"));
//   CHECK_FALSE(f.isJSONFile("file_without_extension"));
//   CHECK_FALSE(f.isJSONFile("document.txt"));
//   CHECK_FALSE(f.isJSONFile("myfile.json.txt"));
//   CHECK(f.isJSONFile("path/to/file.json"));
//   CHECK_FALSE(f.isJSONFile("path/to/document.json.txt"));
//   // should this be valid?
//   CHECK(f.isJSONFile("http://example.com/data.json"));
//   CHECK_FALSE(f.isJSONFile("https://example.com/document.json.txt"));
// }


TEST_CASE("Check if a string is a valid URL", "[Fetch]") {
  Fetch f;

  // Positive examples (valid URLs)
  CHECK(f.isURL("http://example.com"));
  CHECK(f.isURL("https://www.example.com"));

  // Negative examples (not valid URLs)
  CHECK_FALSE(f.isURL("ftp://example.com"));

  // This is find for now, but something to reconsider
  CHECK_FALSE(f.isURL("www.example.com")); // Missing protocol
  CHECK_FALSE(f.isURL("invalid-url")); // Not a URL format
  CHECK_FALSE(f.isURL("path/to/file.json"));  
}


// TEST_CASE("Check if a string is a valid package name", "[Fetch]") {
//   Fetch f;

//   // Positive examples (valid alphanumeric strings)
//   CHECK(f.isPackageName("AlphaNumeric123"));
//   CHECK(f.isPackageName("Valid123"));

//   // Negative examples (not valid alphanumeric strings)
//   CHECK_FALSE(f.isPackageName("Contains Spaces")); // Space is not alphanumeric
//   CHECK_FALSE(f.isPackageName("Special@Characters")); // @ is not alphanumeric
//   CHECK_FALSE(f.isPackageName("123-456")); // - is not alphanumeric
//   CHECK_FALSE(f.isPackageName("")); // Empty string is not alphanumeric
// }
