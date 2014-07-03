/*
 * Base64Codec.cpp
 *
 *  Created on: Apr 27, 2014
 *      Author: orbeus
 */

#include "Base64Codec.h"

#include <iostream>

using namespace std;

namespace rekognition_api {

namespace {

static const string kBase64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/"; // no + in url

static inline bool CheckBase64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static int mod_table[] = {0, 2, 1};

  static inline string url_encode( const char c ) {
    if ( c == '+' ) {
      return string( "%2B" );
    } else if ( c == '/' ) {
      return string( "%2F" );
    } else if ( c == '=' ) {
      return string( "%3D" );
    } else {
      return string( 1, c );
    }
  }

}

Base64Codec::Base64Codec() {
}

Base64Codec::~Base64Codec() {
}

  bool Base64Codec::base64_encode(const string &data, string * output) {
    // http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
    
    size_t input_length = data.size();
    size_t output_length = 4 * ((input_length + 2) / 3);
    
    for (int i = 0, j = 0; i < input_length;) {
      
      uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
      uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
      uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;
      
      uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;
      
      char one = encoding_table[(triple >> 3 * 6) & 0x3F];
      char two = encoding_table[(triple >> 2 * 6) & 0x3F];
      char three = encoding_table[(triple >> 1 * 6) & 0x3F];
      char four = encoding_table[(triple >> 0 * 6) & 0x3F];
      
      (*output) += url_encode( one );
      (*output) += url_encode( two );
      (*output) += url_encode( three );
      (*output) += url_encode( four );
    }
    
    for (int i = 0; i < mod_table[input_length % 3]; i++)
      (*output) += url_encode( '=' );
    
    return true;
  }
  
bool Base64Codec::Encode(const string& input, string* output) {
	if (output == NULL) {
		cerr << "Null output is passed" << endl;
		return false;
	}

	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];
	
	int in_len = input.size();
	const char* bytes_to_encode = &(input[0]);
	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
					+ ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
					+ ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i < 4); i++) {
				const char c = kBase64Chars[char_array_4[i]];

				if (c == '+') {
				  (*output) += "%2B";
				} else if ( c == '/' ) {
				  (*output) += "%2F";
				} else if ( c == '=' ) {
				  (*output) += "%3D";
				} else {
				  (*output) += c;
				}
			}
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4)
				+ ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2)
				+ ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++) {
			const char c = kBase64Chars[char_array_4[i]];
			if (c == '+') {
			  (*output) += "%2B";
			} else if ( c == '/' ) {
			  (*output) += "%2F";
			} else if ( c == '=' ) {
			  (*output) += "%3D";
			} else {
			  (*output) += c;
			}

			/*
			if (c == '+') {
				(*output) += "%2B";
			} else {
				(*output) += c;
			}
			*/
		}

		while ((i++ < 3))
			(*output) += "%3D";

	}

	return true;
}

bool Base64Codec::Decode(const string& input, string* output) {
	cerr << "Decode Not implemented.";
	return false;
}

} /* namespace rekognition_api */
