
#include "async_http_client.h"
#include <iostream>
#include <ostream>
#include <ctype.h>
#include <exception>
#include <boost/regex.hpp>

AsyncHttpClient::AsyncHttpClient(const std::string& server, const std::string& path) : resolver_(io_service), socket_(io_service) {

  /* Parsing port and protocol, if any.
   *
   */
  std::string host = server;
  std::string port = "";

  // check port
  boost::regex port_exp(":\\d+");
  boost::smatch what;
  if (boost::regex_search(host, what, port_exp)) {
    port = what[0];
    host.erase(host.find(port), port.size());
    port.erase(0, 1); // remove ':' symbol
  }

  // Check protocol
  std::string protocol;
  boost::regex proto_exp(".+://");
  boost::smatch proto_what;
  if (boost::regex_search(host, proto_what, proto_exp)) {
    protocol = proto_what[0];
    host.erase(host.find(protocol), protocol.size());
    protocol.resize(protocol.size() - 3); // remove "://" delimeter
  }
  if (protocol != "http" && protocol != "https") {
    throw std::runtime_error("Undefined protocol");
  }
  protocol = protocol.empty()? "http" : protocol;
  https_mode_ = protocol == "https" ? true : false;
  port = port.empty() ? protocol : port;

  std::ostream request_stream(&request_);
  request_stream << "GET " << path << " HTTP/1.0\r\n";
  request_stream << "Host: " << host << "\r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";

  std::cout << "[Parse control] Original server: " << server << "; Host: " << host << "; Port: " << port << std::endl;

  ctx_.set_default_verify_paths();
  boost::asio::ip::tcp::resolver::query query(host, port);
  resolver_.async_resolve(query, boost::bind(&AsyncHttpClient::handleResolve, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator));
}

AsyncHttpClient::~AsyncHttpClient() {

}

void AsyncHttpClient::run() {
  io_service.run();
}

void AsyncHttpClient::handleResolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
  if (!err) {
    if (https_mode_) {
      ssl_socket_.set_verify_mode(boost::asio::ssl::verify_peer);
      ssl_socket_.set_verify_callback(boost::bind(&AsyncHttpClient::verifyCertificate, this, _1, _2));
      boost::asio::async_connect(ssl_socket_.lowest_layer(), endpoint_iterator, boost::bind(&AsyncHttpClient::handleConnect, this, boost::asio::placeholders::error));
    }
    else {
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      socket_.async_connect(endpoint, boost::bind(&AsyncHttpClient::handleConnect, this, boost::asio::placeholders::error, ++endpoint_iterator));
    }
  } else {
    std::cout << "Error: " << err.message() << std::endl;
  }
}

/* Handler for http protocol
 *
 */
void AsyncHttpClient::handleConnect(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
  if (!err) {
    boost::asio::async_write(socket_, request_, boost::bind(&AsyncHttpClient::handleWriteRequest, this, boost::asio::placeholders::error));
  } else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator()) {
    socket_.close();
    boost::asio::ip::tcp::endpoint endpoint  = *endpoint_iterator;
    socket_.async_connect(endpoint, boost::bind(&AsyncHttpClient::handleConnect, this, boost::asio::placeholders::error, ++endpoint_iterator));
  } else {
    std::cout << "Connection error: " << err.message() << std::endl;
  }
}

/* Handler for https protocol
 *
 */
void AsyncHttpClient::handleConnect(const boost::system::error_code &err) {
  if (!err)
  {
    std::cout << "Connect OK " << "\n";
    ssl_socket_.async_handshake(boost::asio::ssl::stream_base::client, boost::bind(&AsyncHttpClient::handleHandshake, this, boost::asio::placeholders::error));
  }
  else
  {
    std::cout << "Connect failed: " << err.message() << "\n";
  }
}

/* Special method for https
 *
 */
void AsyncHttpClient::handleHandshake(const boost::system::error_code &err) {
  if (!err) {
    std::cout << "Handshake OK " << std::endl;
    std::cout << "Request: " << std::endl;
    const char* header = boost::asio::buffer_cast<const char*>(request_.data());
    std::cout << header << std::endl;

    // The handshake was successful. Send the request.
    boost::asio::async_write(ssl_socket_, request_, boost::bind(&AsyncHttpClient::handleWriteRequest, this, boost::asio::placeholders::error));
  }
  else {
    std::cout << "Handshake failed: " << err.message() << "\n";
  }
}

void AsyncHttpClient::handleWriteRequest(const boost::system::error_code& err) {
  if (!err) {
    if (https_mode_){
      boost::asio::async_read_until(ssl_socket_, response_, "\r\n", boost::bind(&AsyncHttpClient::handleReadStatusLine, this, boost::asio::placeholders::error));
    } else {
      boost::asio::async_read_until(socket_, response_, "\r\n", boost::bind(&AsyncHttpClient::handleReadStatusLine, this, boost::asio::placeholders::error));
    }
  } else {
    std::cout << "Write request error: " << err.message() << std::endl;
  }
}

void AsyncHttpClient::handleReadStatusLine(const boost::system::error_code& err) {
  if (!err) {

    // Check that response is OK.
    std::istream response_stream(&response_);

    std::string http_version;
    response_stream >> http_version;
    unsigned int status_code;
    response_stream >> status_code;
    std::string status_message;
    std::getline(response_stream, status_message);
    if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
      std::cout << "Invalid response" << std::endl;
      return;
    }
    if (status_code != 200) {
      std::cout << "Response returned with status code ";
      std::cout << status_code << std::endl;
      return;
    }
    
    if (https_mode_) {
      boost::asio::async_read_until(ssl_socket_, response_, "\r\n\r\n", boost::bind(&AsyncHttpClient::handleReadHeaders, this, boost::asio::placeholders::error));
    } else {
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n", boost::bind(&AsyncHttpClient::handleReadHeaders, this, boost::asio::placeholders::error));
    }

  } else {
    std::cout << "Error: " << err.message() << std::endl;
  }
}

void AsyncHttpClient::handleReadHeaders(const boost::system::error_code& err) {
  if (!err) {

    // Process the response headers.
    std::istream response_stream(&response_);
    std::string header;
    while (std::getline(response_stream, header) && header != "\r") {
      std::cout << header << std::endl;
    }
    std::cout << std::endl;

    // Write whatever content we already have to output.
    if (response_.size() > 0) {
      std::cout << &response_;
    }

    // Read the response headers, which are terminated by a blank line.
    if (https_mode_) {
      boost::asio::async_read(ssl_socket_, response_, boost::asio::transfer_at_least(1), boost::bind(&AsyncHttpClient::handleReadContent, this, boost::asio::placeholders::error));
    } else {
      boost::asio::async_read(socket_, response_, boost::asio::transfer_at_least(1), boost::bind(&AsyncHttpClient::handleReadContent, this, boost::asio::placeholders::error));
    }

  } else {
    std::cout << "Error: " << err.message() << std::endl;
  }
}

void AsyncHttpClient::handleReadContent(const boost::system::error_code& err) {
  if (!err) {

    // Write all of the data that has been read so far.
    std::cout << &response_;

    // Continue reading remaining data until EOF.
    if (https_mode_) {
      boost::asio::async_read(ssl_socket_, response_, boost::asio::transfer_at_least(1), boost::bind(&AsyncHttpClient::handleReadContent, this, boost::asio::placeholders::error));
    } else {
      boost::asio::async_read(socket_, response_, boost::asio::transfer_at_least(1), boost::bind(&AsyncHttpClient::handleReadContent, this, boost::asio::placeholders::error));
    }

  } else if (err != boost::asio::error::eof) {
    std::cout << "Error: " << err.message() << std::endl;
  }
}

bool AsyncHttpClient::verifyCertificate(bool preverified, boost::asio::ssl::verify_context &ctx)
{
  // The verify callback can be used to check whether the certificate that is
  // being presented is valid for the peer. For example, RFC 2818 describes
  // the steps involved in doing this for HTTPS. Consult the OpenSSL
  // documentation for more details. Note that the callback is called once
  // for each certificate in the certificate chain, starting from the root
  // certificate authority.

  // In this example we will simply print the certificate's subject name.
  char subject_name[256];
  X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
  X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
  std::cout << "Verifying " << subject_name << std::endl;

  return preverified;
}
