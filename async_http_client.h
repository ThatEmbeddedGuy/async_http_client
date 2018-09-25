/*
 * =====================================================================================
 *
 *       Filename:  async_http_client.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  20.09.2018
 *       Revision:  1.0
 *       Compiler:  gcc
 *
 *         Author:  Marat G, 
 *   Organization:  
 *
 * =====================================================================================
 */

#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>


class AsyncHttpClient {

  void handleResolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
  void handleConnect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
  void handleConnect(const boost::system::error_code& err);
  void handleHandshake(const boost::system::error_code& err);
  void handleWriteRequest(const boost::system::error_code& err);
  void handleReadStatusLine(const boost::system::error_code& err);
  void handleReadHeaders(const boost::system::error_code& err);
  void handleReadContent(const boost::system::error_code& err);
  bool verifyCertificate(bool preverified, boost::asio::ssl::verify_context& ctx);
  boost::asio::io_service io_service;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
  boost::asio::ssl::context ctx_{boost::asio::ssl::context::sslv23};

  // sockets for https and http protocols
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket_{io_service, ctx_};
  boost::asio::ip::tcp::socket socket_{io_service};

  // https protocol flag
  bool https_mode_ = false;

public:
  AsyncHttpClient(const std::string& server, const std::string& path);
  ~AsyncHttpClient();
  void run();
};
