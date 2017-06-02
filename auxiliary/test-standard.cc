/*~-------------------------------------------------------------------------~~*
 * Copyright (c) 2014 Los Alamos National Security, LLC
 * All rights reserved.
 *~-------------------------------------------------------------------------~~*/

// Boost command-line options
#if defined(ENABLE_BOOST_PROGRAM_OPTIONS)
  #include <boost/program_options.hpp>
  using namespace boost::program_options;
#endif

// This define lets us use the same test driver for gtest and internal
// devel tests.
#if defined(CINCH_DEVEL_TEST)
  #include "cinchdevel.h"
#else
  #include <gtest/gtest.h>
  #include "cinchtest.h"
#endif

//----------------------------------------------------------------------------//
// Implement a function to print test information for the user.
//----------------------------------------------------------------------------//

#if defined(CINCH_DEVEL_TEST)
void print_devel_code_label(std::string name) {
  // Print some test information.
  clog(info) <<
    OUTPUT_LTGREEN("Executing development test " << name) << std::endl;
} // print_devel_code_label
#endif

//----------------------------------------------------------------------------//
// Allow extra initialization steps to be added by the user.
//----------------------------------------------------------------------------//

#if defined(CINCH_OVERRIDE_DEFAULT_INITIALIZATION_DRIVER)
  int driver_initialization(int argc, char ** argv);
#else
  inline int driver_initialization(int argc, char ** argv) { return 0; }
#endif

//----------------------------------------------------------------------------//
// Main
//----------------------------------------------------------------------------//

int main(int argc, char ** argv) {
  
#if !defined(CINCH_DEVEL_TEST)
  // Initialize the GTest runtime
  ::testing::InitGoogleTest(&argc, argv);
#endif

  // Initialize tags to output all tag groups from CLOG
  std::string tags("all");

#if defined(ENABLE_BOOST_PROGRAM_OPTIONS)
  options_description desc("Cinch test options");  

  // Add command-line options
  desc.add_options()
    ("help,h", "produce help message")
    ("tags,t", value(&tags)->implicit_value("0"),
      "--tags=tag1,tag2 --tags by itself will print the available tags.")
    ;

  // parse the command line, allowing unregistered options
  variables_map vm;
  parsed_options parsed = 
    command_line_parser(argc, argv).options(desc).allow_unregistered().run();
  store(parsed, vm);

  notify(vm);

  // gather the unregistered options, if there are any, print a help message
  // and die nicely
  std::vector<std::string> unrecog_options = 
    collect_unrecognized(parsed.options, include_positional);
  
  if ( unrecog_options.size() ) {
    std::cout << std::endl << "Unrecognized options: ";
    for ( int i=0; i<unrecog_options.size(); ++i ) {
      std::cout << unrecog_options[i] << " ";
    }
    std::cout << std::endl << std::endl << desc << std::endl;
    return 1;
  };

  // produce a help message
  if ( vm.count("help") ) {
    std::cout << std::endl << desc << std::endl;
    return 0;
  }
  
#endif // ENABLE_BOOST_PROGRAM_OPTIONS
	
  int result(0);

  if(tags == "0") {
    // Output the available tags
    std::cout << "Available tags (CLOG):" << std::endl;

    for(auto t: clog_tag_map()) {
      std::cout << "  " << t.first << std::endl;
    } // for
  }
  else {
    // Initialize the cinchlog runtime
    clog_init(tags);

    // Call the user-provided initialization function
    driver_initialization(argc, argv);

#if defined(CINCH_DEVEL_TEST)
    // Perform test initialization.
    cinch_devel_code_init(print_devel_code_label);

    // Run the devel test.
    user_devel_code_logic();
#else
    // Get GTest listeners
    ::testing::TestEventListeners& listeners =
      ::testing::UnitTest::GetInstance()->listeners();

    // Adds a listener to the end.  Google Test takes the ownership.
    listeners.Append(new cinch::listener);

    // Run the tests for this target.
    result = RUN_ALL_TESTS();
#endif
	} // if

	return result;
} // main

/*~------------------------------------------------------------------------~--*
 * vim: set tabstop=2 shiftwidth=2 expandtab :
 *~------------------------------------------------------------------------~--*/
