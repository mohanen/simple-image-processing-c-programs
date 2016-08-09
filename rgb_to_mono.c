#include<stdio.h>
#include<stdlib.h>
#define HEADER_SIZE 54
#define CHAR_NULL '\0'
#define COLOR_PALLET_SIZE 1024
#define BMP_SIGNATURE 0x4d42
#define RGB_SIZE 24
#define CHAR_RANGE 256
#define BYTE_PER_PIXEL 3
#pragma pack ( push , 1 )


typedef struct header
{
   unsigned short int   signature;      // type of file 4D42 hex for bmp
   unsigned int         bmp_file_size;
   unsigned short int   reserved1;      // reserved
   unsigned short int   reserved2;      // reserved
   unsigned int         offset;         // offset to start image data in bytes

}HEADER;

typedef struct info_header
{
   unsigned int         info_header_size;
   int                  image_width , image_height;     // in pixels
   unsigned short int   no_of_planes;                   // must be 1
   unsigned short int   bits_per_pixel;                 // can be 1 , 4 , 8 , 24
   unsigned int         compression_type;               //   0 -none , 1 - RLE-8 , 2 - RLE-4
   unsigned int         image_data_size;                // including padding
   int                  horizontal_resolution, vertical_resolution;
   unsigned int         no_of_colors;
   unsigned int         no_of_imp_colors;

}INFO_HEADER;

typedef struct COLOR
{
   unsigned char   blue;
   unsigned char   green;
   unsigned char   red;
   unsigned char   unused;
}COLOR_PALLET;

#pragma pack(pop)


unsigned char* covert_to_mono ( unsigned char *data_buffer ,int file_size , INFO_HEADER bmp_info_header);
void print_header(HEADER bmp_header , INFO_HEADER bmp_info_header );
int to_file (unsigned char *out_buffer , HEADER header , INFO_HEADER info_header , COLOR_PALLET *bgr_pallete , int file_size);
void get_color_pallet (COLOR_PALLET *bgr_pallete);
char* get_file_name ();
int xscanf ();

int main ()
{
   int file_size , padding = 0;
   HEADER bmp_header;
   INFO_HEADER bmp_info_header;
   COLOR_PALLET *bgr_pallete = malloc (sizeof( COLOR_PALLET ) * CHAR_RANGE);
   unsigned char *data_buffer , *out_buffer ;
   char *file_name ;
   FILE *in_file = NULL;

   file_name = get_file_name ();
   in_file =  fopen ( file_name , "rb" );
   if (in_file == NULL)
   {
      printf("file opening failed\n");
      return 1;
   }

   if ( fread ( &bmp_header , sizeof ( HEADER ) , 1 , in_file) < 1 )
   {
      printf("Some issue in reading file\n");
      return 0;
   }

   if ( fread ( &bmp_info_header , sizeof ( INFO_HEADER ) , 1 , in_file) < 1 )
   {
      printf("Some issue in reading file\n");
      return 1;
   }
   print_header( bmp_header , bmp_info_header );
   //checking if the image is bmp or not
   if ( bmp_header.signature != BMP_SIGNATURE )
   {
      printf("no a bmp\n");
      return 1;
   }

   if ( bmp_info_header.bits_per_pixel != RGB_SIZE )
   {
      printf("already in greysacle\n");
      return 1;
   }

   if ( fseek ( in_file , bmp_header.offset , SEEK_SET ) == -1 )
   {
      printf("file operation failed\n");
      return 1;
   }

   file_size = bmp_info_header.image_data_size ;
   //based on file_size allocate memory
   data_buffer = malloc (sizeof (char) * file_size + 1 );
   if (data_buffer == NULL)
   {
     printf("malloc Failed\n");
     return 1;
   }
   *(data_buffer  + file_size) = CHAR_NULL;
   //read the data into buffer
   if ( fread ( data_buffer , sizeof(char) , file_size , in_file ) < file_size )
   {
     printf("Some issue in reading file\n");
     fclose (in_file);
     free (data_buffer);
     return 1;
   }

   if ( fclose ( in_file ) == 0  )
   {
      printf("File not closing\n" );
   }

   out_buffer = covert_to_mono ( data_buffer , file_size , bmp_info_header);

   if ( ( bmp_info_header.image_width * 3 ) % 4 != 0)
   padding = 4 - ( ( bmp_info_header.image_width * 3 ) % 4 );

   //change the header details
   bmp_header.offset = COLOR_PALLET_SIZE + HEADER_SIZE;
   bmp_header.bmp_file_size = HEADER_SIZE + ( bmp_info_header.image_width * bmp_info_header.image_height ) + ( padding * bmp_info_header.image_height ) + COLOR_PALLET_SIZE;
   bmp_info_header.bits_per_pixel = 8;
   bmp_info_header.image_data_size =( bmp_info_header.image_width * bmp_info_header.image_height ) + ( padding * bmp_info_header.image_height );

   print_header( bmp_header , bmp_info_header );

   //get bgr pallete
   get_color_pallet ( bgr_pallete );

   to_file (out_buffer, bmp_header , bmp_info_header , bgr_pallete , bmp_info_header.image_data_size) ;


   free (data_buffer);
   data_buffer = NULL;
   free ( out_buffer );
   out_buffer = NULL;
   free (bgr_pallete);
   bgr_pallete = NULL;

   return 0;
}
/*
this function makes the 3 byte rgb to 1 byte grey scale image
*/
unsigned char* covert_to_mono ( unsigned char *data_buffer , int file_size , INFO_HEADER bmp_info_header )
{
   int i = 0 , j = 0 , k = 0 , padding = 0 , count = 1;
   padding = 4 - ( ( bmp_info_header.image_width * 3 ) % 4 );

   unsigned char *converted_buffer = calloc ( (file_size/ BYTE_PER_PIXEL) + ( padding * bmp_info_header.image_height) , sizeof(char));
   if (converted_buffer == NULL)
   {
      printf("malloc Failed\n");
      return NULL;
   }


   if (padding == 4)
   {
      for ( i = 0 , j = 0 ; i <= file_size ; i += BYTE_PER_PIXEL , j ++)
      {
         *(converted_buffer + j) = ( *(data_buffer + i) *.20 ) +  (*(data_buffer + i + 1) * .70 ) +  (*(data_buffer + i + 2) * .10) ;
         //*(converted_buffer + j) = ( *(data_buffer + i)  +  *(data_buffer + i + 1)  +  *(data_buffer + i + 2) ) / BYTE_PER_PIXEL  ;
      }
   }
   else
   {
      printf("\n\n\nimage has padding\n");
      *(converted_buffer ) = ( *(data_buffer ) *.20 ) +  (*(data_buffer + 1) * .70 ) +  (*(data_buffer + 2) * .10) ;

      for ( i = BYTE_PER_PIXEL , j = 1 ; i < file_size ; i += BYTE_PER_PIXEL , j ++)
      {
         *(converted_buffer + j) = ( *(data_buffer + i) *.20 ) +  (*(data_buffer + i + 1) * .70 ) +  (*(data_buffer + i + 2) * .10) ;

        if ( j %  ( bmp_info_header.image_width + padding ) == 0 )
        {
          for ( k = 1 ; k <= padding  ; k++ )
          {
             *(converted_buffer + j + k) =  *(data_buffer + i + 2 + k) ;
          }

          j += padding ;
          i += padding ;
        }

      }
   }

   return converted_buffer ;
}


/*
prints the header information
*/
void print_header(HEADER bmp_header ,INFO_HEADER bmp_info_header )
{
   printf("\n\n\n\nsignature\t\t%02x\n", bmp_header.signature);
   printf("size of bmp\t\t%d\n", bmp_header.bmp_file_size);
   printf("offset\t\t%d\n",bmp_header.offset );

   printf("header size\t\t%d\n", bmp_info_header.info_header_size);
   printf("image_width\t\t%d\n",bmp_info_header.image_width );
   printf("image_height\t\t%d\n", bmp_info_header.image_height);
   printf("no_of_planes\t\t%d\n", bmp_info_header.no_of_planes);
   printf("bits_per_pixel\t\t%d\n", bmp_info_header.bits_per_pixel );
   printf("compression_type\t\t%d\n",bmp_info_header.compression_type );
   printf("size of image\t\t%d\n", bmp_info_header.image_data_size);
   printf("horizontal_resolution\t\t%d\n", bmp_info_header.horizontal_resolution );
   printf("vertical_resolution\t\t%d\n", bmp_info_header.vertical_resolution);
   printf("no_of_colors\t\t%d\n",bmp_info_header.no_of_colors );
   printf("no_of_imp_colors\t\t%d\n", bmp_info_header.no_of_imp_colors);

}


/*
write compressed data to the file
*/

int to_file ( unsigned char *out_buffer , HEADER header , INFO_HEADER info_header , COLOR_PALLET * bgr_pallete , int file_size)
{
  FILE *file ;
  file = fopen ( "mono" , "wb");
  if (file == NULL)
  {
    printf("File creation failed\n" );
    return 0;
  }

    printf("->%ld\n",ftell(file) );
    fwrite ( &header , sizeof(HEADER) , 1 , file );
    printf("->%ld\n",ftell(file) );
    fwrite ( &info_header , sizeof(INFO_HEADER) , 1 , file );
    printf("->%ld\n",ftell(file) );
    fwrite ( bgr_pallete , sizeof(COLOR_PALLET) , CHAR_RANGE , file );
    printf("->%ld\n",ftell(file) );
    fwrite ( out_buffer , sizeof(char) , file_size , file );
    printf("->%ld\n",ftell(file) );

    fclose (file);
    return 1;
}

/*
returns char ponter with string in it
*/
char* get_file_name ()
{
   printf("Enter file_name size\n");
   int file_name_size = xscanf ();

   char *name_format = calloc ( sizeof(char) , 10 );
   if ( name_format == NULL )
   {
      printf("malloc failed\n");
      return 0;
   }

   char *file_name = calloc ( sizeof(char) , file_name_size);
   if ( file_name == NULL )
   {
      printf("malloc failed\n");
      return 0;
   }


   printf("Enter file_name : only the chars upto specifies length will be taken\n");
   sprintf( name_format , "%c%ds" , '%',file_name_size + 1);
   scanf( name_format , file_name );
   while ( getchar() != '\n' );
   printf("you entered :\t%s\n",file_name );

   return file_name ;
}

/*
return integer from input
*/
int xscanf ()
{
  int temp ;
  while ( scanf("%d", &temp ) == 0 )
  {
    while (getchar() != '\n');
    printf("\nEnter a integer \n");
  }
  return temp;
}
/*
genrates the color pallete for the grey scale image
*/

void get_color_pallet (COLOR_PALLET *bgr_pallete)
{
   int i;
   for ( i = 0; i < CHAR_RANGE; i++) {
      ( bgr_pallete + i )->blue     = i ;
      ( bgr_pallete + i )->green    = i ;
      ( bgr_pallete + i )->red      = i ;
      ( bgr_pallete + i )->unused   = CHAR_NULL;
   }
}
