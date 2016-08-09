#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define HEADER_SIZE 54
#define CHAR_NULL '\0'
#define BMP_SIGNATURE 0x4d42
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


#pragma pack(pop)


char* get_file_name ();
unsigned char* zoom ( unsigned char *data_buffer , INFO_HEADER bmp_info_header);
void print_header(HEADER bmp_header ,INFO_HEADER bmp_info_header );
int to_file (unsigned char *out_buffer , HEADER header , INFO_HEADER info_header , int file_size);
int xscanf ();

int main ()
{
   int   file_size;
   HEADER bmp_header;
   INFO_HEADER bmp_info_header;
   unsigned char *data_buffer , *out_buffer ;
   char *file_name ;
   FILE *in_file = NULL;

   if ( ( file_name = get_file_name () ) == NULL )
      return 1;

   in_file =  fopen ( file_name , "rb" );
   if (in_file == NULL)
   {
      printf("file opening failed\n");
      return 1;
   }
   if ( fread ( &bmp_header , sizeof ( HEADER ) , 1 , in_file) < 1 )
   {
      printf("Some issue in reading file\n");
      return 1;
   }

   if ( fread ( &bmp_info_header , sizeof ( INFO_HEADER ) , 1 , in_file) < 1 )
   {
      printf("Some issue in reading file\n");
      return 1;
   }

   if ( bmp_header.signature != BMP_SIGNATURE )
   {
      printf("no a bmp\n");
      return 1;
   }


   if ( bmp_info_header.bits_per_pixel < 8 )
   {
      printf("already in greysacle\n");
      return 1;
   }

   print_header( bmp_header , bmp_info_header );

   fseek ( in_file , bmp_header.offset , SEEK_SET );


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

   fclose ( in_file );

   printf("\nThe image is of size %d x %d \n",bmp_info_header.image_width , bmp_info_header.image_height );

   out_buffer = zoom ( data_buffer , bmp_info_header );

   print_header( bmp_header ,  bmp_info_header );

   to_file (out_buffer, bmp_header , bmp_info_header, file_size ) ;

   free ( out_buffer );
   out_buffer = NULL;

   return 0;
}

/*
this function gets the coordinates and frame width height of the image to be zoomed
*/
unsigned char* zoom ( unsigned char *data_buffer , INFO_HEADER bmp_info_header)
{
   int x , y , frame_width , frame_height , i , j , k , l , m , byte_per_pizel ;
   int xpixel_multiplier = 0 , ypixel_multiplier = 0 , crop_data_size = 0  , zoom_value;

   unsigned char *cropped_data , *out_buffer ;
   printf("Enter the x  coordinates\n");
   while ( (x = xscanf()) >= bmp_info_header.image_width  || x <= 0)
   {
      printf("Enter valid x coordinate\n");
   }

   printf("Enter the y  coordinates\n");
   while ( (y = xscanf()) >= bmp_info_header.image_height || y <= 0)
   {
      printf("Enter valid y coordinate\n");
   }

   byte_per_pizel =  bmp_info_header.bits_per_pixel / 8 ;



   printf("Enter zoom value like 2x ,3x, 4x ..etc , without x\n");

   zoom_value = xscanf ();

   if ( x < ( bmp_info_header.image_width / ( 2 * zoom_value ) )  )
      x =  bmp_info_header.image_width / ( 2 * zoom_value ) ;

   if ( ( bmp_info_header.image_width - x )  <  ( bmp_info_header.image_width / ( 2 * zoom_value ) ) )
      x = bmp_info_header.image_width - ( bmp_info_header.image_width / ( 2 * zoom_value ) );

   if ( y < ( bmp_info_header.image_height / ( 2 * zoom_value ) ) )
      y =  bmp_info_header.image_height / ( 2 * zoom_value ) ;

   if ( ( bmp_info_header.image_height - y )  <  ( bmp_info_header.image_height / ( 2 * zoom_value ) ) )
      y = bmp_info_header.image_height - ( bmp_info_header.image_height / ( 2 * zoom_value ) );


   frame_height =  bmp_info_header.image_height  / zoom_value;
   frame_width = ( frame_height * bmp_info_header.image_width ) / bmp_info_header.image_height ;
   printf("the frame width will be %d then\n" , frame_width);

   //gives the value of how many times to duplicate the pixel to fit current resolution
   xpixel_multiplier =  bmp_info_header.image_width  /  frame_width ;
   ypixel_multiplier =  bmp_info_header.image_height / frame_height;

   cropped_data = malloc (  ( byte_per_pizel ) * frame_height * frame_width );

   x -= ( frame_width / 2 );
   y -= (frame_height / 2 );

   printf("x-%d\ty-%d\tfw-%d\tfh-%d\t\n",x,y,frame_width , frame_height  );

   //crop the image
   for ( i = y  ; i < y + frame_height ; i++ )
   {
      for ( j = x  ; j < x + frame_width  ; j ++ )
      {
         for ( k = 0 ; k < byte_per_pizel ; k++ )
         {
            *( cropped_data + crop_data_size ) = *(data_buffer + j * byte_per_pizel + ( i * bmp_info_header.image_width * byte_per_pizel) + k );
            crop_data_size ++;
         }
      }
   }


   //data_buffer has no use from now on
   //so memset it to null and use as out buffer
   out_buffer = data_buffer ;
   memset ( out_buffer , CHAR_NULL , bmp_info_header.image_data_size + 1);

   //duplicate pixels of cropped image to match the size of original image
   for ( i = 0 ; i < frame_height ; i++ )
   {
      for ( k = 0 ; k < ypixel_multiplier ; k ++ )
      {
         for ( j = 0 ; j < frame_width ; j ++  )
         {
            for ( l = 0 ; l < xpixel_multiplier ; l++ )
            {
               for ( m = 0 ; m < byte_per_pizel ; m++ )
               {
                  *( out_buffer + j * byte_per_pizel * xpixel_multiplier + l * byte_per_pizel + ( i * bmp_info_header.image_width * byte_per_pizel * ypixel_multiplier) + ( k * bmp_info_header.image_width * byte_per_pizel ) + m ) = *(cropped_data + j  * byte_per_pizel + ( i * frame_width * byte_per_pizel ) + m );
               }
            }
         }
      }
   }

   return out_buffer;


}

/*
prints the header information
*/
void print_header(HEADER bmp_header ,INFO_HEADER bmp_info_header )
{
   printf("\n\n\n\nsignature                  %02x\n", bmp_header.signature);
   printf("\nsize of bmp                %d\n", bmp_header.bmp_file_size);
   printf("\noffset                     %d\n",bmp_header.offset );

   printf("\nheader size                %d\n", bmp_info_header.info_header_size);
   printf("\nimage_width                %d\n",bmp_info_header.image_width );
   printf("\nimage_height               %d\n", bmp_info_header.image_height);
   printf("\nno_of_planes               %d\n", bmp_info_header.no_of_planes);
   printf("\nbits_per_pixel             %d\n", bmp_info_header.bits_per_pixel );
   printf("\ncompression_type           %d\n",bmp_info_header.compression_type );
   printf("\nsize of image              %d\n", bmp_info_header.image_data_size);
   printf("\nhorizontal_resolution      %d\n", bmp_info_header.horizontal_resolution );
   printf("\nvertical_resolution        %d\n", bmp_info_header.vertical_resolution);
   printf("\nno_of_colors               %d\n",bmp_info_header.no_of_colors );
   printf("\nno_of_imp_colors           %d\n", bmp_info_header.no_of_imp_colors);

}


/*
write compressed data to the file
*/

int to_file (unsigned char *out_buffer , HEADER header , INFO_HEADER info_header , int file_size)
{
  FILE *file ;
  file = fopen ( "zoomed.bmp" , "wb");
  if (file == NULL)
  {
    printf("File creation failed\n" );
    return 1;
  }

    printf("->%ld\n",ftell(file) );
    fwrite ( &header , sizeof(HEADER) , 1 , file );
    printf("->%ld\n",ftell(file) );
    fwrite ( &info_header , sizeof(INFO_HEADER) , 1 , file );
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
      return NULL;
   }

   char *file_name = calloc ( sizeof(char) , file_name_size + 1);
   if ( file_name == NULL )
   {
      printf("malloc failed\n");
      return NULL;
   }


   printf("Enter file_name : only the chars upto specifies length will be taken\n");
   sprintf( name_format , "%c%ds" , '%',file_name_size + 1);
   scanf( name_format , file_name );
   while ( getchar() != '\n' );
   *(file_name + file_name_size ) = CHAR_NULL;
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
