/*
 * cmatrix-bitmap 
 * see readme.md or /docs for information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PNG_HEADER_SIZE 8
void processFile(const char*);
int checkFile(FILE *);

/*
  We will hold char (byte) ptr to array of values.
  We will avoid the problem of int being 32bit or 64 bit depending on env.
  Dynamically allocate based on defined size of chunk
*/

typedef struct ihdr_data_t{
  int width;            //4 bytes, stored as int
  int height;           //4 bytes, stored as int
  char bitDepth;          //1 byte
  char colorType;         //1 byte
  char compressionMethod; //1 byte
  char filterMethod;      //1 byte
  char interlaceMethod;   //1 byte
}IHDR_DATA;

//this chunk must precede IDAT chunk, can only be one chunk
//for color type 3, PLTE == required. PLTE can appear in 2 and 6 as a type as well.

typedef struct plte_data_t{
  char red;   //(0 = black, 255 = red)
  char green; //(0 = black, 255 = green)
  char blue;  //(0 = black, 255 = blue)
}PLTE_DATA;

//multiple IDAT chunks are allowed. Actual image data ins encoded here.
typedef struct idat_data_t{

}IDAT_DATA;



//TODO: Fix the chunk_t structure to hold char arrays. oofta
typedef struct chunk_t{
  int length; //the length of the chunks data field
  char* type;   //4 bytes
  char * data;  //0 + length
  // union {
  //   //chunk can contain no data
  //   IHDR_DATA ihdrData;
  //   PLTE_DATA plteData;
  //   IDAT_DATA idatData;
  // }data;
  char * crc;   //4 bytes
} Chunk;


const char PNGHEADER_BYTES[8] = {137,80,78,71,13,10,26,10};//decimal form of defined PNG header 8 bytes
const char IEND_TYPE_BYTES[4] = {73,69,78,68};//decimal ascii for IEND
const char IHDR_TYPE_BYTES[4] = {73,72,68,82};//decmimal ascii for IHDR
const char IDAT_TYPE_BYTES[4] = {73,68,65,84};//decimal ascii for IDAT

/*
 * Entry point of test application
 * Receive number of arguments passed cli, ptr to array of arguments
 */  
int main(int argc, char* argv[]){

    const char * filename;
    //second arg passed is filename, thrid = overlay png
    if(argc >= 2){
      filename = argv[1];
      printf("Filename passed: %s\n", filename);
      processFile(filename);
    }
    else if(argc < 2){
      printf("Error. Supply png file to overlay.\n");
      exit(1);
    }

    return 0;

}

/*
 *  This function checks for PNG header bytes indicating the file is actually
 *  a PNG file.  
 *  @param: *pngFile
 *  @return: integer 0 == false, integer 1 == true
 */ 
int checkFile(FILE * pngFile){
    /*
          we are processing PNG files so skip/ check the header for valid file info.
      First 8 bytes of PNG file : http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

      The first eight bytes of a PNG file always contain the following (decimal) values:

      137 80 78 71 13 10 26 10

      This signature indicates that the remainder of the file contains a single PNG image,
       consisting of a series of chunks beginning with an IHDR chunk and ending with an IEND chunk.

    */

    char pngHeaderBuffer[PNG_HEADER_SIZE];
      //seek to beginning of the file
    fseek(pngFile,0,SEEK_SET); //SEEK_SET == 0
    /* Read and display data
       place data into pngHeaderBuffer,Size of 1 byte , PNG_HEADER_SIZE = 8 elements to read, from pngFile ptr
    */ 
    fread(pngHeaderBuffer, 1, PNG_HEADER_SIZE, pngFile);

    //check if the read header data matches defined png format
    int cmpVal = strncmp(PNGHEADER_BYTES, pngHeaderBuffer, 8); //first 8 chars
    if(cmpVal == 0){
      return 1;
    }
    else{
      system("setterm -bold on");
      printf("Not PNG file: %d.\n", cmpVal);
      system("setterm -bold off");
      return 0;
    }
}

/*
 *  This function processes pngFile into heap organized by its chunks.  
 * 
 *  @param: *filename
 *  @return: null
 */
void processFile(const char * filename){
    int numBytes = 0;
    FILE *pngFile;

    char byteBuffer[1];
    char chunkFieldBuffer[4]; //buffer for reading field by field on chunks
    char * lengthPtr = chunkFieldBuffer;

    pngFile = fopen(filename,"rb"); //read binary value of png file.

    if(!pngFile){
      //we failed to properly open filestream
      system("setterm -bold on");
      printf("FATAL: \n");
      system("setterm -bold off");
      printf("Error opening png file. Check formatting.\n");
      exit(EXIT_FAILURE);
    }

    if(checkFile(pngFile)){
      //seek to desired post-png header
      fseek(pngFile,8, SEEK_SET);

      /*
          Source: http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

          PNG file description
          Length
          A 4-byte unsigned integer giving the number of bytes in the chunk's data field. 
          The length counts only the data field, not itself, the chunk type code, or the CRC. 
          Zero is a valid length. Although encoders and decoders should treat the length as unsigned, 
          its value must not exceed 231 bytes


          Chunk Type
          A 4-byte chunk type code. For convenience in description and in examining PNG files, 
          type codes are restricted to consist of uppercase and lowercase ASCII letters 
          (A-Z and a-z, or 65-90 and 97-122 decimal). However, encoders and decoders must treat the codes as fixed 
          binary values, not character strings. For example, it would not be correct to represent the type code 
          IDAT by the EBCDIC equivalents of those letters. Additional naming conventions for chunk types are discussed 
          in the next section.

          Chunk Data
          The data bytes appropriate to the chunk type, if any. This field can be of zero length.

          CRC
          A 4-byte CRC (Cyclic Redundancy Check) calculated on the preceding bytes in the chunk, including the chunk 
          type code and chunk data fields, but not including the length field. The CRC is always present, even for 
          chunks containing no data. See CRC algorithm.
      */

     /*
        Approach: Read the data length of the chunk, seek ahead and check the chunk type, will only care about the IDAT chunk and looking for specific
        binary value indicators with respect to the dimensions of the image (obtained from IHDR chunk).

     */
      
      //read 4 bytes to figure out length of data
      //int guaranteed to be at least 32 bit, int = ok
      fread(chunkFieldBuffer,4,1,pngFile);
      int chunkLength = chunkFieldBuffer[3]+chunkFieldBuffer[2]*256+chunkFieldBuffer[1]*(256*2)+chunkFieldBuffer[0]*(256*3);
      
      //Obtain IHDR chunk
      Chunk * IHDR_CHUNK = malloc(sizeof(Chunk)*1); //allocate a new chunk for critical IHDR chunk ptr
      /*
        The IHDR chunk must appear FIRST. It contains:

          Width:              4 bytes
          Height:             4 bytes
          Bit depth:          1 byte
          Color type:         1 byte
          Compression method: 1 byte
          Filter method:      1 byte
          Interlace method:   1 byte
      */
      //IHDR_CHUNK->length = malloc(sizeof(char)*4); //32 bit length 
      IHDR_CHUNK->length = chunkLength;
      IHDR_CHUNK->type = malloc(sizeof(char)*5); //32 bit type code (checking bits to determine types)
      IHDR_CHUNK->data = malloc(sizeof(char)*chunkLength+1); //we know how much memory to allocate from the chunks 4 byte length chunk
      IHDR_CHUNK->crc = malloc(sizeof(char)*5); //32 bit crc
      
      printf("IHDR chunk length: %d\n", IHDR_CHUNK->length);
      //read next 4 bytes to get chunk type
      fread(chunkFieldBuffer,4,1,pngFile);
      strncpy(IHDR_CHUNK->type, chunkFieldBuffer+'\0', 5); 
      printf("chunk type: %s\n",IHDR_CHUNK->type);

      char dataBuffer[chunkLength];
      fread(dataBuffer,chunkLength,1, pngFile);
      printf("image width : %d pixels.\n", dataBuffer[3]+dataBuffer[2]*256+dataBuffer[1]*(256*2)*dataBuffer[0]*(256*3));
      printf("image height : %d pixels.\n", dataBuffer[7]+dataBuffer[6]*256+dataBuffer[5]*(256*2)*dataBuffer[4]*(256*3));
      printf("bit depth: %d\n", dataBuffer[8]);
      printf("color type: %d\n", dataBuffer[9]);
      printf("compression method: %d\n", dataBuffer[10]);
      printf("filter method: %d\n", dataBuffer[11]);
      printf("interlace method: %d\n", dataBuffer[12]);
      strncpy(IHDR_CHUNK->data, dataBuffer+'\0', chunkLength+1);

      //read the crc field
      fread(chunkFieldBuffer,4,1,pngFile);
      strncpy(IHDR_CHUNK->crc,chunkFieldBuffer+'\0',5);
      printf("IHDR Chunk crc: %s\n", IHDR_CHUNK->crc);

      /*
        Begin next chunk, PLTE or IDAT
      */
     //get the legnth of chunk
     fread(chunkFieldBuffer,4,1,pngFile);
     chunkLength = chunkFieldBuffer[3]+chunkFieldBuffer[2]*256+chunkFieldBuffer[1]*(256*2)+chunkFieldBuffer[0]*(256*3);
     printf("Decimal byte values buff 3: %d buff2: %d buff1: %d buff0: %d\n", chunkFieldBuffer[3], chunkFieldBuffer[2], chunkFieldBuffer[1], chunkFieldBuffer[0]);

     //load chunk into heap, testing so we "know" what types beforehand
     Chunk * IDAT_CHUNK = malloc(sizeof(Chunk)*1); //allocate a new chunk for critical IHDR chunk ptr
     IDAT_CHUNK->length = chunkLength;
     IDAT_CHUNK->type = malloc(sizeof(char)*5); //32 bit type code (checking bits to determine types)
     IDAT_CHUNK->data = malloc(sizeof(char)*chunkLength+1); //we know how much memory to allocate from the chunks 4 byte length chunk
     IDAT_CHUNK->crc = malloc(sizeof(char)*5); //32 bit crc
     printf("IDAT chunk length: %d\n", IDAT_CHUNK->length);

     //read next 4 bytes to get chunk type
      fread(chunkFieldBuffer,4,1,pngFile);
      strncpy(IDAT_CHUNK->type, chunkFieldBuffer+'\0', 5); 
      printf("IDAT chunk type: %s\n",IDAT_CHUNK->type);

      char idatBuffer[chunkLength];
      fread(idatBuffer,chunkLength,1, pngFile);
      strncpy(IDAT_CHUNK->data, idatBuffer+'\0', chunkLength+1);
      printf("IDAT chunk data: %s\n", IDAT_CHUNK->data);


      //TODO: Figure out how to identify regions that are colored/contain
      //values we want to annotate onto cmatrix

      //looking through zlib compression. edit..



      //read the crc field
      fread(chunkFieldBuffer,4,1,pngFile);
      strncpy(IDAT_CHUNK->crc,chunkFieldBuffer+'\0',5);
      printf("IDAT Chunk crc: %s\n", IDAT_CHUNK->crc);

      /*
        Begin next chunk, IEND for testIMG 32x10_square.png
        data field should == 0
      */
      //get the legnth of chunk
     fread(chunkFieldBuffer,4,1,pngFile);
     chunkLength = chunkFieldBuffer[3]+chunkFieldBuffer[2]*256+chunkFieldBuffer[1]*(256*2)+chunkFieldBuffer[0]*(256*3);
     Chunk * IEND_CHUNK = malloc(sizeof(Chunk)*1); //allocate a new chunk for critical IHDR chunk ptr
     IEND_CHUNK->length = chunkLength;
     IEND_CHUNK->type = malloc(sizeof(char)*5); //32 bit type code (checking bits to determine types)
     IEND_CHUNK->data = malloc(sizeof(char)*chunkLength+1); //we know how much memory to allocate from the chunks 4 byte length chunk
     IEND_CHUNK->crc = malloc(sizeof(char)*5); //32 bit crc
     printf("IEND chunk length: %d\n", IEND_CHUNK->length);

      //read next 4 bytes to get chunk type
      fread(chunkFieldBuffer,4,1,pngFile);
      strncpy(IEND_CHUNK->type, chunkFieldBuffer+'\0', 5); 
      printf("IEND chunk type: %s\n",IEND_CHUNK->type);

      //would get chunk data here but should be == 0
      // char iendBuffer[chunkLength];
      // fread(iendBuffer,chunkLength,1, pngFile);
      // strncpy(IEND_CHUNK->data, iendBuffer+'\0', chunkLength+1);
      // printf("IEND chunk data: %s\n", IEND_CHUNK->data);

      //read the crc field
      fread(chunkFieldBuffer,4,1,pngFile);
      strncpy(IEND_CHUNK->crc,chunkFieldBuffer+'\0',5);
      printf("IEND Chunk crc: %s\n", IEND_CHUNK->crc);



      //Read 1 bytes at a time until EOF
      //these are extra bytes, after IEND crc we are done. 
      while(fread(byteBuffer,1,1,pngFile)){
          printf("Chunk Buffer: %x \n", byteBuffer[0] & 0xff);
          numBytes++;
      }
      printf("Number Byte remaining in file: %d\n", numBytes);

      fclose(pngFile); //close the file
      //free allocated memory
      free(IHDR_CHUNK->crc);
      free(IHDR_CHUNK->data);
      free(IHDR_CHUNK->type);
      free(IHDR_CHUNK);
      free(IDAT_CHUNK->crc);
      free(IDAT_CHUNK->data);
      free(IDAT_CHUNK->type);
      free(IDAT_CHUNK);
      free(IEND_CHUNK->crc);
      free(IEND_CHUNK->data);
      free(IEND_CHUNK->type);
      free(IEND_CHUNK);

    }

}
