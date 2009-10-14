/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file src/io.c
//!
//! @brief File reading and writing.
//!
//! Functions that mostly deal with reading and writing files.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>
#include <curl/curl.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionaries.h>


char     save_path[FILENAME_MAX]    = { '\0' };


//!
//! \brief Writes a file using the given text and write mode
//!
//! This function is an extension of the save as/append buttons.
//! Thus, the program currently only uses the write and append write modes.
//! The save path is gathered using some's build in function for a dialog, and the
//! path is saved for as long as the program is running, so it doesn't need to be
//! asked a second time.
//!
//! @param write_mode A constant char representing the write mode to be used (w,a)
//! @param text A char pointer to some text to save.
//!
void gw_io_write_file(const char* write_mode, gchar *text)
{
    if (save_path[0] != '\0')
    {
      //Get the data for the file
      gchar *text_ptr;
      int status = 0;

      text_ptr = &text[0];

      //Write it
      FILE* fd;
      fd = fopen(save_path, write_mode);

      while (*text_ptr != '\0' && (status = fputc(*text_ptr, fd)) != EOF)
        text_ptr++;

      if (status != EOF) fputc('\n', fd);
      //if (status != EOF) fputc('\0', fd);

      fclose(fd);
      fd = NULL;

      //Cleanup
      text_ptr = NULL;
    }
}


//!
//! \brief Checks to see if rsync is available
//!
//! The variable that this checks is set at compile time, so if the program
//! was compiled when rsync wasn't available, the option will be preminiently
//! disabled.
//!
//! @return The status of the existance of rsync
//!
gboolean gw_io_check_for_rsync()
{
    gboolean rsync_exists;
    rsync_exists = ( RSYNC != NULL     &&
                     strlen(RSYNC) > 0 &&
                     g_file_test(RSYNC, G_FILE_TEST_IS_EXECUTABLE)
                   );
    return rsync_exists;
}


//!
//! \brief Copies a file and creates a new one using the new encoding
//!
//! This function is made to be risilient to errors unlike the built in easy
//! to use g_convert.  It will skip over any characters it has problems converting.
//!
//! @param source_path The source file to change the encoding on.
//! @param target_path The place to save the new file with the new encoding.
//! @param source_encoding The encoding of the source file.
//! @param target_encoding THe wanted encoding in the new file to be created.
//!
//! @return The status of the conversion opertaion
//!
gboolean gw_io_copy_with_encoding( char *source_path,     char *target_path,
                                  char *source_encoding, char *target_encoding,
                                  GError **error)
{
    FILE* readfd = NULL;
    readfd = fopen (source_path, "r");
    if (readfd == NULL) exit(0);

    FILE* writefd = NULL;
    writefd = fopen (target_path, "w");
    if (writefd == NULL) exit(0);

    int length = MAX_LINE;
    char buffer[length];
    char output[length];
    gsize inbytes_left, outbytes_left;
    char *inptr, *outptr;
    char prev_inbytes = 0;

    GIConv conv = g_iconv_open (target_encoding, source_encoding);
    while (fgets(buffer, length, readfd) != NULL)
    {
      inptr = buffer; outptr = output;
      inbytes_left = length; outbytes_left = length;
      while (inbytes_left && outbytes_left && g_iconv (conv, &inptr, &inbytes_left, &outptr, &outbytes_left) != -1)
      {
        if (prev_inbytes == inbytes_left)
        {
          inptr++;
          inbytes_left--;
        }
        prev_inbytes = inbytes_left;
        inptr = inptr + strlen(inptr) - inbytes_left;
        outptr = outptr + strlen(outptr) - outbytes_left;
      }
      fwrite(output, 1, strlen(output), writefd); 
    }
    g_iconv_close (conv);

    close(readfd);
    close(writefd);

    return TRUE;
}


static size_t write_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}
 

static size_t read_func(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread(ptr, size, nmemb, stream);
}


gboolean gw_io_download_dictionary_file(char *source_path, char *save_path, gpointer func, gpointer data)
{
    CURL *curl;
    CURLcode res;
    FILE *outfile = NULL;

    curl = curl_easy_init();

    if(curl == NULL) return FALSE;

    outfile = fopen(save_path, "w");

    curl_easy_setopt(curl, CURLOPT_URL, source_path);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_func);
    if (func != NULL && data != NULL)
    {
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, func);
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, data);
    }

    res = curl_easy_perform(curl);
     
    fclose(outfile);
    curl_easy_cleanup(curl);

    //If curl didn't write a clean file for some reason, end the function here
    if (res != 0) g_remove(save_path);

    return (res == 0);
}


gboolean gw_io_delete_dictionary_file(GwDictInfo* file)
{
    printf("  ");
    printf(gettext("Removing %s..."), file->long_name);
    printf("\n");

    printf("*   ");
    printf(gettext("Removing %s..."), file->path);
    if (g_remove(file->path) == 0)
      printf("Success\n");
    else
      printf("Failed\n");

    printf("*   ");
    printf(gettext("Removing %s..."), file->gz_path);
    if (g_remove(file->gz_path) == 0)
      printf("Success\n");
    else
      printf("Failed\n");

    printf("*   ");
    printf(gettext("Removing %s..."), file->sync_path);
    if (g_remove(file->sync_path) == 0)
      printf("Success\n");
    else
      printf("Failed\n");

    return TRUE;
}


gboolean gw_io_copy_dictionary_file(char *source_path, char *target_path, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);

    char *contents = NULL;
    gssize length;
    if ( g_file_get_contents(source_path, &contents, &length, NULL) == FALSE ||
         g_file_set_contents(target_path, contents, length, NULL) == FALSE     )
    {
      printf("Error while copying the file...cleaning up.\n");
      remove(target_path);
      const char *message = gettext("File copy error");
      if (error != NULL)
        *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
      return FALSE;
    }
    if (contents != NULL) g_free(contents);

    return TRUE;
}


//Creates a single dictionary containing both the radical dict and kanji dict
gboolean gw_io_create_mix_dictionary(char *mpath, char *kpath, char *rpath)
{
    printf(gettext("  Recreating mixed dictionary..."));

    FILE* kanji_file =  fopen(kpath, "r");
    FILE* output_file = fopen(mpath, "w");
    FILE *radicals_file = NULL;

    char radicals_input[MAX_LINE];
    char* radicals_ptr = NULL;

    char kanji_input[MAX_LINE];
    char* kanji_ptr = NULL;

    char output[MAX_LINE * 2];
    char* output_ptr = NULL;

    char* temp_ptr;

    //Loop through the kanji file
    while ( fgets(kanji_input, MAX_LINE, kanji_file) != NULL )
    {
      if(kanji_input[0] == '#') continue;

      kanji_ptr = kanji_input;
      output_ptr = output;

      //1. Copy the kanji character from the kanji line
      while (*kanji_ptr != ' ')
      {
        *output_ptr = *kanji_ptr;
        output_ptr++;
        kanji_ptr++;
      }

      //2. Find the relevent radical line and insert it if available
      radicals_file = fopen(rpath, "r");
      while ( fgets(radicals_input, MAX_LINE, radicals_file) != NULL )
      {
        //Check for a match
        temp_ptr = kanji_input;
        radicals_ptr = radicals_input;
        while (*radicals_ptr != ' ' && *radicals_ptr == *temp_ptr)
        {
          temp_ptr++;
          radicals_ptr++;
        }

        //If a match is found...
        if (*radicals_ptr == ' ')
        {
          //Skip over the colon
          radicals_ptr++;
          radicals_ptr++;
   
          //Copy the data
          while (*(radicals_ptr + 1) != '\0')
          {
            *output_ptr = *radicals_ptr;
            output_ptr++;
            radicals_ptr++;
          }

          break;
        }
      }
      fclose(radicals_file);

      //3. Copy the rest of the kanji line to output
      while (*kanji_ptr != '\0')
      {
        *output_ptr = *kanji_ptr;
        output_ptr++;
        kanji_ptr++;
      }

      //4. Close off the string and write it to the file
      *output_ptr = '\0';
      fputs(output, output_file);
      output[0] = '\0';
    }

    //Cleanup
    fclose(kanji_file);
    fclose(output_file);

    printf(gettext("success\n"));

    return TRUE;
}


gboolean gw_io_split_places_from_names_dictionary (char *spath, char *npath, char *ppath)
{
    /*
      Current composition of the Enamdic dictionary
      ----------------------------------------------
      s - surname (138,500)
      p - place-name (99,500)
      u - person name, either given or surname, as-yet unclassified (139,000) 
      g - given name, as-yet not classified by sex (64,600)
      f - female given name (106,300)
      m - male given name (14,500)
      h - full (family plus given) name of a particular person (30,500)
      pr - product name (55)
      co - company name (34)
      ---------------------------------------------
    */
    int eflags_exist = REG_EXTENDED | REG_NOSUB;
    //Setup the regular expression for Places
    regex_t re_place_line;
    char *place_pattern = "([\\(,])((p)|(st))([\\),])";
    if (regcomp(&re_place_line, place_pattern, eflags_exist) != 0)
    {
      printf("A problem occured while setting the regular expression for place\n");
      return FALSE; 
    }

    //Setup the regular expression for Names
    regex_t re_name_line;
    char *name_pattern = "([\\(,])((s)|(u)|(g)|(f)|(m)|(h)|(pr)|(co))([\\),])";
    if (regcomp(&re_name_line, name_pattern, eflags_exist) != 0)
    {
      printf("A problem occured while setting the regular expression for name\n");
      return FALSE;
    }

    //Buffer
    char buffer[MAX_LINE];

    g_remove (npath);
    g_remove (ppath);

    //Setup the file descriptors
    FILE *input_stream = NULL;
    input_stream = fopen(spath, "r");
    FILE *places_stream;
    places_stream = fopen(ppath, "w");
    FILE *names_stream = NULL;
    names_stream = fopen(npath, "w");
    

    //Error checking
    int  place_write_error = 0;
    long total_place_lines = 0;
    int  name_write_error  = 0;
    long total_name_lines  = 0;

    //Start writing the child files
    while ( fgets(buffer, MAX_LINE, input_stream) != NULL &&
            place_write_error != EOF &&
            name_write_error  != EOF                            )
    {
      if (places_stream != NULL && regexec(&re_place_line, buffer, 1, NULL, 0) == 0)
      {
        place_write_error = fputs(buffer, places_stream);
        total_place_lines++;
      }
      if (names_stream != NULL && regexec(&re_name_line, buffer, 1, NULL, 0) == 0)
      {
        name_write_error =  fputs(buffer, names_stream);
        total_name_lines++;
      }
    }

    //Check for failure
    ;

    //Cleanup
    if (input_stream != NULL)
    {
      fclose(input_stream);
      input_stream = NULL;
    }

    if (places_stream != NULL)
    {
      fclose(places_stream);
      places_stream = NULL;
    }
    regfree(&re_place_line);

    if (names_stream != NULL)
    {
      fclose(names_stream);
      names_stream = NULL;
    }
    regfree(&re_name_line);


    if (total_place_lines == 0 && ppath != NULL)
      g_remove(ppath);
    if (total_name_lines == 0 && npath != NULL)
      g_remove(npath);

    return (place_write_error != EOF && name_write_error != EOF);
}


gboolean gw_io_gunzip_dictionary_file(char *path, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    gboolean success;

    char command[FILENAME_MAX]; 
    strcpy(command, GUNZIP);
    strcat(command, " ");
    strcat(command, path);

    success = (system(command) == 0);

    if (success == FALSE) {
      g_remove(path);
      const char *message = gettext("gunzip error");
      if (error != NULL)
        *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
    }

    return success;
} 


gboolean gw_io_unzip_dictionary_file(char *path, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    gboolean success;

    char extraction_directory[FILENAME_MAX];
    strcpy(extraction_directory, path);
    *strrchr (extraction_directory, G_DIR_SEPARATOR) = '\0';


    char command[FILENAME_MAX]; 
    strcpy(command, UNZIP);
    strcat(command, " ");
    strcat(command, path);
    strcat(command, " -d ");
    strcat(command, extraction_directory);

    success = (system(command) == 0);

    if (success == FALSE) {
      g_remove(path);
      const char *message = gettext("gunzip error");
      if (error != NULL)
        *error = g_error_new_literal (quark, GW_FILE_ERROR, message);
    }
    return success;
}


int gw_io_get_total_lines_for_path (char *path)
{
    //Calculate the number of lines in the dictionary
    char line[MAX_LINE];
    int total_lines = 0;
    FILE *fd = fopen (path, "r");
    if (fd != NULL)
    {
      while (fgets(line, MAX_LINE, fd) != NULL)
        total_lines++;
      fclose(fd);
    }
    return total_lines;
}

