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
#include <locale.h>
#include <libintl.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <curl/curl.h>

#include <gwaei/backend.h>


static gchar *_savepath = NULL;


//!
//! @brief Creates a savepath that is used with the save/save as functions
//!
//! @param PATH a path to save to
//!
void gw_io_set_savepath (const gchar *PATH)
{
    if (_savepath != NULL)
    {
      g_free (_savepath);
      _savepath = NULL;
    }

    if (PATH != NULL)
      _savepath = g_strdup (PATH);
}

const gchar* gw_io_get_savepath ()
{
  return _savepath;
}


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
void gw_io_write_file (const char* path, const char* mode, gchar *text, GwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity checks
    g_assert (path != NULL && mode != NULL && text != NULL);
    if (*error != NULL) return;

    //Declarations
    gchar *text_ptr;
    int status;
    FILE* fd;

    //Initializations
    status = 0;
    text_ptr = &text[0];
    fd = fopen(_savepath, mode);

    while (*text_ptr != '\0' && (status = fputc(*text_ptr, fd)) != EOF)
      text_ptr++;

    if (status != EOF) fputc('\n', fd);

    //Cleanup
    fclose(fd);
    fd = NULL;
    text_ptr = NULL;
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
gboolean gw_io_copy_with_encoding (const char *source_path, const char *target_path,
                                   const char *source_encoding, const char *target_encoding,
                                   GwIoProgressCallback cb, gpointer data, GError **error   )
{
    if (*error != NULL) return FALSE;


    FILE* readfd = NULL;
    readfd = fopen (source_path, "r");
    if (readfd == NULL) exit(0);

    FILE* writefd = NULL;
    writefd = fopen (target_path, "w");
    if (writefd == NULL) exit(0);

    int length = GW_IO_MAX_FGETS_LINE;
    char buffer[length];
    char output[length];
    gsize inbytes_left, outbytes_left;
    char *inptr, *outptr;
    char prev_inbytes = 0;

    size_t written;
    GIConv conv = g_iconv_open (target_encoding, source_encoding);
    while (fgets(buffer, length, readfd) != NULL)
    {
      inptr = buffer; outptr = output;
      inbytes_left = length; outbytes_left = length;
      while (inbytes_left && outbytes_left)
      {
        if (g_iconv (conv, &inptr, &inbytes_left, &outptr, &outbytes_left) == -1) break;

        //Force increment if there is something wrong
        if (prev_inbytes == inbytes_left)
        {
          inptr++;
          inbytes_left--;
        }
        //Normal operation
        prev_inbytes = inbytes_left;
        inptr = inptr + strlen(inptr) - inbytes_left;
        outptr = outptr + strlen(outptr) - outbytes_left;
      }
      written = fwrite(output, 1, strlen(output), writefd); 
    }
    g_iconv_close (conv);

    fclose(readfd);
    fclose(writefd);

    return TRUE;
}


//!
//! \brief Private function made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t _libcurl_write_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}
 

//!
//! @brief Private function made to be used with gw_io_download_file
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t _libcurl_read_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread (ptr, size, nmemb, stream);
}

//!
//! @brief Private struct made to be used with gw_io_download_file
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static int _libcurl_update_progress (void  *custom,
                                     double dltotal,
                                     double dlnow,
                                     double ultotal,
                                     double ulnow   )
{
    //Declarations
    GwIoProgressCallbackWithData *cbwdata;
    GwIoProgressCallback cb;
    gpointer data;
    double percent;
    
    //Initializations
    cbwdata = (GwIoProgressCallbackWithData*) custom;
    cb = cbwdata->cb;
    data = cbwdata->data;
    if (dltotal == 0.0)
      percent = 0.0;
    else
      percent = dlnow / dltotal;

    //Update the interface
    return cb (percent, data);
}




//!
//! \brief Downloads a file using libcurl
//!
//! @param source_path String of the source url
//! @param target_path String of the path to save the file locally
//! @param cb Pointer to a function to update
//! @param data gpointer to data to pass to the function pointer
//! @param error Error handling
//!
gboolean gw_io_download_file (char *source_path, char *target_path, GwIoProgressCallback cb,
                              gpointer data, GError **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    GQuark quark;
    CURL *curl;
    CURLcode res;
    FILE *outfile;
    char *message;
    GwIoProgressCallbackWithData cbwdata;

    //Initializations
    curl = curl_easy_init ();
    outfile = fopen(target_path, "wb");
    cbwdata.cb = cb;
    cbwdata.data = data;

    if (curl != NULL || outfile != NULL)
    {
      curl_easy_setopt(curl, CURLOPT_URL, source_path);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
      curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _libcurl_write_func);
      curl_easy_setopt(curl, CURLOPT_READFUNCTION, _libcurl_read_func);

      if (cb != NULL)
      {
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, _libcurl_update_progress);
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &cbwdata);
      }

      res = curl_easy_perform(curl);
    }
     
    fclose(outfile);
    curl_easy_cleanup(curl);

    if (res != 0)
    {
      g_remove (target_path);

      if (error != NULL) {
        message = gettext(curl_easy_strerror(res));
        quark = g_quark_from_string (GW_IO_ERROR);
        *error = g_error_new_literal (quark, GW_IO_DOWNLOAD_ERROR, message);
      }
    }

    return (res == 0);
}


//!
//! \brief Copies a local file to another local location
//!
//! @param source_path String of the source url
//! @param target_path String of the path to save the file locally
//! @param error Error handling
//!
gboolean gw_io_copy_file (char *source_path, char *target_path, GwIoProgressCallback cb, gpointer data, GError **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    GQuark quark;
    char *contents;
    gsize length;

    //Initalizations
    quark = g_quark_from_string (GW_IO_ERROR);
    contents = NULL;

    //Copy the file
    if (g_file_get_contents(source_path, &contents, &length, NULL) == FALSE ||
        g_file_set_contents(target_path, contents, length, NULL) == FALSE     )
    {
      remove(target_path);
      if (error != NULL) *error = g_error_new_literal (quark, GW_IO_COPY_ERROR, gettext("File copy error"));
      return FALSE;
    }

    //Cleanup
    if (contents != NULL) g_free (contents);

    return TRUE;
}


//!
//! @brief Creates a single dictionary containing both the radical dict and kanji dict
//! @param mpath Mix dictionary path to write to
//! @param kpath Kanjidic dictionary path
//! @param rpath raddic dictionary path
//! @param error pointer to a GError to write errors to
//!
gboolean gw_io_create_mix_dictionary (const char *output_path, 
                                      const char *kanji_dictionary_path, 
                                      const char *radicals_dictionary_path, 
                                      GwIoProgressCallback cb,
                                      gpointer data,
                                      GError **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    FILE *output_file, *kanji_file, *radicals_file;
    char radicals_input[GW_IO_MAX_FGETS_LINE];
    char kanji_input[GW_IO_MAX_FGETS_LINE];
    char output[GW_IO_MAX_FGETS_LINE * 2];
    char *radicals_ptr, *kanji_ptr, *output_ptr, *temp_ptr;

    //Initializations
    kanji_file =  fopen(kanji_dictionary_path, "r");
    radicals_file = fopen(radicals_dictionary_path, "r");
    output_file = fopen(output_path, "w");
    radicals_ptr = NULL;
    kanji_ptr = NULL;
    output_ptr = NULL;

    //Loop through the kanji file
    while (fgets(kanji_input, GW_IO_MAX_FGETS_LINE, kanji_file) != NULL )
    {
      if (kanji_input[0] == '#') continue;

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
      rewind (radicals_file);
      while ( fgets(radicals_input, GW_IO_MAX_FGETS_LINE, radicals_file) != NULL)
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

    return TRUE;
}


gboolean gw_io_split_places_from_names_dictionary (const char *names_dictionary_path, 
                                                   const char* names_output_path,
                                                   const char* places_output_path,
                                                   GwIoProgressCallback cb,
                                                   gpointer data,
                                                   GError **error                    )
{
    if (*error != NULL) return FALSE;
/*
      buffer = g_strdup (names);
      ptr = buffer + strlen(buffer) 
      while (g_utf8_get_char (ptr) != g_utf8_get_char (G_DIR_SEPARATOR_S) && ptr != buffer)
        ptr = g_utf8_prev_char (ptr);
      strcpy(ptr, "Places");


      g_free (buffer);
*/



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

    /*
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
    char buffer[GW_IO_MAX_FGETS_LINE];

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
    while ( fgets(buffer, GW_IO_MAX_FGETS_LINE, input_stream) != NULL &&
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
    */
    return FALSE;
}


//!
//! \brief Gunzips a file
//!
//! @param path String representing the path of the file to gunzip
//! @param error Error handling
//!
gboolean gw_io_gunzip_file (char *path, GwIoProgressCallback cb, gpointer data, GError **error)
{
    if (*error != NULL) return FALSE;

    GQuark quark;
    quark = g_quark_from_string (GW_IO_ERROR);
    gboolean success;

    char *command = g_strdup_printf ("%s %s", GUNZIP, path);
    printf("command: %s\n", command);

    success = (system(command) == 0);

    g_free (command);
    command = NULL;

    if (success == FALSE) {
      g_remove(path);
      const char *message = gettext("gunzip error");
      if (error != NULL) *error = g_error_new_literal (quark, GW_IO_DECOMPRESSION_ERROR, message);
    }

    return success;
} 


//!
//! \brief Unzips a file
//!
//! @param path String representing the path of the file to unzip
//! @param error Error handling
//!
gboolean gw_io_unzip_file (char *path, GwIoProgressCallback cb, gpointer data, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GW_IO_ERROR);
    gboolean success;

    char *extraction_directory = (char*) malloc(strlen(path + 1) * sizeof(char));
    strcpy(extraction_directory, path);
    char* end = strrchr (extraction_directory, G_DIR_SEPARATOR);
    *end = '\0';

    char *command = g_strdup_printf ("%s %s %s %s %s", UNZIP, "-o", path, "-d", extraction_directory);

    success = (system(command) == 0);

    g_free (command);
    command = NULL;
    free(extraction_directory);
    extraction_directory = NULL;

    if (success == FALSE) {
      g_remove(path);
      const char *message = gettext("gunzip error");
      if (error != NULL) *error = g_error_new_literal (quark, GW_IO_DECOMPRESSION_ERROR, message);
    }
    return success;
}


//!
//! \brief Returns the total lines in a dictionary file for processing purposes.
//!
//! @param path The string representing path to the dictionary file
//!
int gw_io_get_total_lines_for_path (char *path)
{
    //Calculate the number of lines in the dictionary
    char line[GW_IO_MAX_FGETS_LINE];
    int total_lines = 0;
    FILE *fd = fopen (path, "r");
    if (fd != NULL)
    {
      while (fgets(line, GW_IO_MAX_FGETS_LINE, fd) != NULL)
        total_lines++;
      fclose(fd);
    }
    return total_lines;
}


//!
//! @brief Gets a list of the currently installed dictionaries as an array of strings.
//!
//! The format will be ENGINE/FILENAME and the array is null terminated.  Both the array
//! and string themselves must be freed after.
//!
//! @returns An array of strings that must be freed.  We recommend g_strfreev() from glib
//!
char** gw_io_get_dictionary_file_list ()
{
    //Declarations and initializations
    int engine;
    GDir *dir;
    const char* enginename;
    const char* filename;
    const char *directory;
    const int MAX = GW_DICTLIST_MAX_DICTIONARIES;
    char** atoms = (char**) malloc((MAX + 1) * sizeof(int));
    int i = 0;
    GError *error = NULL;

    //Go through each engine folder looking for dictionaries
    for (engine = 0; engine < GW_ENGINE_TOTAL && i < MAX; engine++)
    {
      enginename = gw_util_get_engine_name (engine);
      if ((directory = gw_util_get_directory_for_engine (engine)) != NULL)
      {
        dir = g_dir_open (directory, 0, NULL);

        //Look for files in the directory and stop if we reached the max for the program
        while (dir != NULL && (filename =  g_dir_read_name (dir)) != NULL && i < MAX)
        {
          atoms[i] = g_strdup_printf("%s/%s", enginename, filename);
          i++;
        }
        g_dir_close(dir);
      }
    }
    atoms[i] = NULL;

    return atoms;
}


