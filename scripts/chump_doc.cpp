//-----------------------------------------------------------------------------
// name: chump_doc.cpp
// desc: chump page documentation generator for packages
//
// author: Nick Shaheed (nshaheed@ccrma.stanford.edu)
//         Ge Wang (ge@ccrma.stanford.edu)
// date: Spring 2025
//-----------------------------------------------------------------------------
#include "package.h"
#include "package_directory.h"
#include "package_list.h"

#include <iostream>
#include <sstream>

// c++ namespace setup
namespace fs = std::filesystem;
using std::optional;
using std::string;

// function prototypes
string generate_page_HTML( Package p );
string generate_mainIndex_HTML( PackageList p );




//-----------------------------------------------------------------------------
// name: main()
// desc: entry point
//-----------------------------------------------------------------------------
int main( int argc, const char** argv )
{
    // check arguments
    if( argc != 3 ) {
        std::cerr << "chump_doc [packages dir] [output dir]" << std::endl;
        return -1;
    }

    // packages dir
    fs::path packages_path = argv[1];
    fs::path packages_subdir = packages_path / "packages";
    // output dir
    fs::path output_dir = argv[2];

    // check packages subdir
    if( !fs::exists( packages_subdir ) || !fs::is_directory( packages_subdir ) ) {
        std::cout << "no 'packages' dir found in " << packages_path
            << " make sure you pointed to the chump-packages repo correctly"
            << std::endl;
        return -1;
    }

    // check output dir
    if( !fs::exists( output_dir ) ) {
        std::cout << "output dir '" << output_dir << "' not found" << std::endl;
        return -1;
    }

    // a vector packages
    std::vector<Package> packages;

    // iterate over contents of packages subdir
    for( auto const & path : fs::directory_iterator{ packages_subdir } )
    {
        // if not a directory, move on to next entry
        if( !fs::is_directory( path ) )
            continue;

        // NOTE: each directory corresponds to a package

        // grab the package.json
        fs::path pkg_path = path.path() / "package.json";
        // std::cout << pkg_path << std::endl;

        // check package.json file
        if( !fs::exists( pkg_path ) ) {
            std::cerr
                << "package definition '" << pkg_path
                << "' not found; are you in the chump-packages/packages directory?"
                << std::endl;
            continue;
        }

        // read package from package.json
        Package pkg = read_package( pkg_path );
        // populate versions of package
        populate_versions( &pkg, path );
        // add to vector of packages
        packages.push_back( pkg );
    }

    // a package list
    PackageList package_list( packages );
    // create output dir packages directory
    fs::create_directory( output_dir / "packages" );

    // iterate over package list
    for( auto const & p : package_list.get_packages() )
    {
        // the package info "chump page"
        fs::path filename = p.name + ".html";
        // path to the chump page
        fs::path pkg_path = output_dir / "packages" / filename;
        // open the file
        std::ofstream out( pkg_path );
        // generate the chump page for the package
        out << generate_page_HTML( p );
        // close the file
        out.close();
    }

    // main index file
    fs::path idx_path = output_dir / "index.html";
    // open the file
    std::ofstream out( idx_path );
    // output the package
    out << generate_mainIndex_HTML( package_list );
    // close the file
    out.close();
}




//-----------------------------------------------------------------------------
// name: generate_HTML()
// desc: generate HTML chump page
//-----------------------------------------------------------------------------
string generate_page_HTML( Package p )
{
    std::stringstream ss;

    ss << "<html>";
    ss << "<title>" << p.name << "</title>";
    ss << "<body>";

    ss << "<h1>" << p.name << "</h1>";

    ss << "<p> install command: chump install " << p.name << "</p>";

    // ss << "<p> authors: " << p.

    ss << "<p>" << p.description << "</p>";

    ss << "<p> Homepage: "
        << "<a href=\"" << p.homepage << "\">" << p.homepage << "</a>"
        << "</p>";
    ;
    ss << "<p> Repository: "
        << "<a href=\"" << p.repository << "\">" << p.repository << "</a>"
        << "</p>";

    ss << "<p> License: " << p.license << "</p>";

    // Current versions (mac, windows, linux)
    ss << "<p>"
        << "Current versions:"
        << "</p>";

    optional<PackageVersion> linux = p.latest_version( "linux" );
    optional<PackageVersion> win = p.latest_version( "windows" );
    optional<PackageVersion> mac = p.latest_version( "mac" );

    ss << "<table>";
    if( linux )
        ss << "<tr>"
        << "<th>"
        << "linux"
        << "</th>"
        << "<th>" << linux.value().getVersionString() << "</th>"
        << "</tr>";
    if( win )
        ss << "<tr>"
        << "<th>"
        << "windows"
        << "</th>"
        << "<th>" << win.value().getVersionString() << "</th>"
        << "</tr>";
    if( mac )
        ss << "<tr>"
        << "<th>"
        << "mac"
        << "</th>"
        << "<th>" << mac.value().getVersionString() << "</th>"
        << "</tr>";
    ss << "</table>";

    ss << "</body>";
    ss << "</html>";

    return ss.str();
}




//-----------------------------------------------------------------------------
// name: generate_mainIndex_HTML()
// desc: generate main packages index in HTML
//-----------------------------------------------------------------------------
string generate_mainIndex_HTML( PackageList pkg_list )
{
    std::stringstream ss;

    ss << "<html>";
    ss << "<title>"
        << "ChuMP"
        << "</title>";
    ss << "<body>";

    ss << "<h1>"
        << "ChuMP (the ChucK Manager of Packages)"
        << "</h1>";
    ss << "<h2>"
        << "Package Directory"
        << "</h2>";

    ss << "<table>";
    for( auto const& p : pkg_list.get_packages() ) {
        ss << "<tr>"
            << "<th>"
            << "<a href=\"./packages/" << p.name << ".html\">" << p.name << "</a>"
            << "</th>"
            << "<th>" << p.description << "</th>"
            << "</tr>";
    }
    ss << "</table>";

    return ss.str();
}
