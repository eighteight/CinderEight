//
//  Util.h
//  OpenniStream
//
//  Created by Vladimir Gusev on 6/24/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OpenniStream_Util_h
#define OpenniStream_Util_h
void findLocations(std::string sample, char findIt)
{
    std::vector<int> characterLocations;
    for(int i =0; i < sample.size(); i++)
        if(sample[i] == findIt)
            std::cout<<sample[i]<<" "<<i<<std::endl;
}

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    return split(s, delim, elems);
}

std::string string_compress(std::string const& s) {
	
	unsigned int sourceSize = s.size();	
	const char * source = s.c_str();
	
	unsigned long dsize = sourceSize + (sourceSize * 0.1f) + 16;
	char * destination = new char[dsize];
	
	int result = compress((unsigned char *)destination, &dsize, (const unsigned char *)source, sourceSize);
	
	if(result != Z_OK)
		std::cout << "Compress error occured! Error code: " << result << std::endl; 
	
    return std::string(destination, dsize);
}

void string_inflate_deflate(){
    char a[500] = "Hello World!qwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwerqwer";
    char b[500];
    char c[500];
    
    // deflate
    // zlib struct
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = (uInt)strlen(a)+1; // size of input, string + terminator
    defstream.next_in = (Bytef *)a; // input char array
    defstream.avail_out = (uInt)sizeof(b); // size of output
    defstream.next_out = (Bytef *)b; // output char array
    
    deflateInit(&defstream, Z_DEFAULT_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);
    
    // This is one way of getting the size of the output
    printf("Deflated size is: %lu\n", (char*)defstream.next_out - b);
    
    // inflate
    // zlib struct
    z_stream infstream;
    infstream.zalloc = Z_NULL;
    infstream.zfree = Z_NULL;
    infstream.opaque = Z_NULL;
    infstream.avail_in = (uInt)((char*)defstream.next_out - b); // size of input
    infstream.next_in = (Bytef *)b; // input char array
    infstream.avail_out = (uInt)sizeof(c); // size of output
    infstream.next_out = (Bytef *)c; // output char array
    
    inflateInit(&infstream);
    inflate(&infstream, Z_NO_FLUSH);
    inflateEnd(&infstream);
    
    printf("Inflate:\n%lu\n%s\n", strlen(c), c);
}

int string_compress(std::string const& s, unsigned long dsize, char* destination) {
	
	unsigned int sourceSize = s.size();	
	const char * source = s.c_str();
	
	int result = compress((unsigned char *)destination, &dsize, (const unsigned char *)source, sourceSize);
	
	if(result != Z_OK)
		std::cout << "Compress error occured! Error code: " << result << std::endl; 
	
    return result;
}

int string_uncompress(std::string const& s, unsigned long len, char* destination) {
	
	unsigned int sourceSize = s.size();
	const char * source = s.c_str();
	
	int result = uncompress((unsigned char *)destination, &len, (const unsigned char *)source, sourceSize);
    
	return result;
}

std::string string_uncompress(std::string const& s, unsigned long len) {
	
	unsigned int sourceSize = s.size();
	const char * source = s.c_str();
	
	char * destination = new char[len];
	
	int result = uncompress((unsigned char *)destination, &len, (const unsigned char *)source, sourceSize);
	
	if(result != Z_OK)
		std::cout << "Uncompress error occured! Error code: " << result << std::endl; 
	
	return destination;
}


#endif
