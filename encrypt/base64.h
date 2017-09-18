#pragma once

#include <string>
namespace base
{
	 /**
       * Base64-encodes the input according to RFC 3548.
       * @param input The data to encode.
       * @return The encoded string.
       */
	const std::string encode64( const std::string& input );
     /**
       * Base64-decodes the input according to RFC 3548.
       * @param input The encoded data.
       * @return The decoded data.
       */
   const std::string decode64( const std::string& input );
}