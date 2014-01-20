#include<iostream>
using namespace std;

// cat ./games.bin | base64 -w 0 | ./bin2c > ./games.cpp

char escaped[32][4] = {
	"0",
	"x01",
	"x02",
	"x03",
	"x04",
	"x05",
	"x06",
	"a",
	"b",
	"x09",
	"n",
	"x0B",
	"f",
	"r",
	"x0E",
	"x0F",
	"x00",
	"x01",
	"x02",
	"x03",
	"x04",
	"x05",
	"x06",
	"x07",
	"x08",
	"t",
	"x0A",
	"v",
	"x0C",
	"x0D",
	"x0E",
	"x0F",
};

int main(int argc, char* arv[])
{
	const uint maxLineLength = 512;
	uint lineLength = 0;
	
	cout << "const char binData[] =" << endl;
	cout << "\t\"";
	while(cin.good()) {
		unsigned char c = cin.get();
		if(c < 0x20)
			cout << "\\" << escaped[c];
		else if(c == 0x22)
			cout << "\\\"";
		else if(c == 0x5c)
			cout << "\\\\";
		else if(c < 0x7f)
			cout << c;
		else
			cout << "\\x" << hex << int(c);
		++lineLength;
		if(lineLength > maxLineLength) {
			cout << "\"" << endl << "\t\"";
			lineLength = 0;
		}
	}
	cout << "\";" << endl;
}

/*

static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;

}
string base64_decode(const string& encoded)
{
	static const string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int in_len = encoded.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	uint8 blockIn[4];
	uint8 blockOut[3];
	std::string result;

	while(in_len-- && ( encoded[in_] != '=')) {
		blockIn[i++] = encoded[in_];
		in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				blockIn[i] = base64_chars.find(blockIn[i]);
			
			// Decode block
			blockOut[0] = (blockIn[0] << 2) + ((blockIn[1] & 0x30) >> 4);
			blockOut[1] = ((blockIn[1] & 0xf) << 4) + ((blockIn[2] & 0x3c) >> 2);
			blockOut[2] = ((blockIn[2] & 0x3) << 6) + blockIn[3];
			
			for (i = 0; (i < 3); i++)
				result += blockOut[i];
			i = 0;
		}
	}
	
	if (i) {
		for (j = i; j <4; j++)
			blockIn[j] = 0;
		for (j = 0; j <4; j++)
			blockIn[j] = base64_chars.find(blockIn[j]);
		
		blockOut[0] = (blockIn[0] << 2) + ((blockIn[1] & 0x30) >> 4);
		blockOut[1] = ((blockIn[1] & 0xf) << 4) + ((blockIn[2] & 0x3c) >> 2);
		blockOut[2] = ((blockIn[2] & 0x3) << 6) + blockIn[3];
		
		for (j = 0; (j < i - 1); j++)
			result += blockOut[j];
	}
	
	return result;
}
*/
