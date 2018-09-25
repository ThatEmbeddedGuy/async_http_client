/*
 * =====================================================================================
 *
 *       Filename:  main.cpp
 *
 *    Description:  test project for asynchronous http client
 *
 *        Version:  1.0
 *        Created:  20.09.2018 17:40:09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Marat G
 *   Organization:  
 *
 * =====================================================================================
 */
#include <exception>
#include <iostream>
#include <boost/asio.hpp>
#include "async_http_client.h"

int main(int argc, char* argv[]) {

  try {
    if (argc < 3) {
      std::cout << "Usage: https_client <server> <path>\n" << std::endl;
      argv[1] = const_cast<char *>(std::string("localhost:8000").c_str());
      argv[2] = const_cast<char *>(std::string("/test.cpp").c_str());
      //return 1;
    }
    AsyncHttpClient client(argv[1], argv[2]);
    client.run();
  } catch (std::exception& e) {
    std::cout << "Exception: " << e.what() << std::endl;
  }

  return 0;
}

