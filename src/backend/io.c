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


char save_path[500] = { '\0' };


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
/*
gboolean gw_io_check_for_rsync ()
{
    gboolean rsync_exists;
    rsync_exists = ( RSYNC != NULL     &&
                     strlen(RSYNC) > 0 &&
                     g_file_test(RSYNC, G_FILE_TEST_IS_EXECUTABLE)
                   );
    return rsync_exists;
}
*/


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
static size_t libcurl_write_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fwrite(ptr, size, nmemb, stream);
}
 

//!
//! \brief Private function made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
static size_t libcurl_read_func (void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return fread (ptr, size, nmemb, stream);
}


//!
//! \brief Private struct made to be used with gw_io_download_file
//!
typedef struct libcurl_callback_func_with_data {
    int (*callback_function) (char*, int, gpointer); //!< function pointer
    gpointer data;                                   //!< gpointer data
} libcurl_callback_func_with_data; 


//!
//! \brief Private struct made to be used with gw_io_download_file
//!
//! @param ptr TBA
//! @param size TBA
//! @param nmemb TBA
//! @param stream TBA
//!
int libcurl_update_progressbar (void   *data,
                                double  dltotal,
                                double  dlnow,
                                double  ultotal,
                                double  ulnow   )
{
    int percent = 0;
    if (dltotal != 0.0)
      percent = (int) (dlnow / dltotal * 100.0);
    libcurl_callback_func_with_data *curldata = (libcurl_callback_func_with_data*) data;
    return (curldata->callback_function) (NULL, percent, curldata->data);
}


//!
//! \brief Downloads a file using libcurl
//!
//! @param source_path String of the source url
//! @param save_path String of the path to save the file locally
//! @param func Pointer to a function to update
//! @param data gpointer to data to pass to the function pointer
//! @param error Error handling
//!
gboolean gw_io_download_file (char *source_path, char *save_path,
                              int (*callback_function) (char*, int, gpointer), gpointer data, GError **error)
{
  /*
    if (*error != NULL) return FALSE;

    CURL *curl;
    CURLcode res;
    FILE *outfile = NULL;
    GwDictInfo *di = (GwDictInfo*) data;

    curl = curl_easy_init ();

    if (curl == NULL) return FALSE;

    outfile = fopen(save_path, "wb");

    curl_easy_setopt(curl, CURLOPT_URL, source_path);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, libcurl_write_func);
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, libcurl_read_func);

    libcurl_callback_func_with_data libcurl_data;
    libcurl_data.callback_function = callback_function;
    libcurl_data.data = data;

    if (callback_function != NULL)
    {
      curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
      curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, libcurl_update_progressbar);
      curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &libcurl_data);
    }

    res = curl_easy_perform(curl);
     
    fclose(outfile);
    curl_easy_cleanup(curl);

    if (res != 0)
    {
      g_remove(save_path);
      if (di->status != GW_DICT_STATUS_CANCELING)
      {
        GQuark quark;
        quark = g_quark_from_string (GW_IO_ERROR);
        const char *message = gettext(curl_easy_strerror(res));
        if (error != NULL) *error = g_error_new_literal (quark, GW_IO_DOWNLOAD_ERROR, message);
      }
    }

    return (res == 0);
    */
}


//!
//! \brief Copies a local file to another local location
//!
//! @param source_path String of the source url
//! @param save_path String of the path to save the file locally
//! @param error Error handling
//!
gboolean gw_io_copy_file (char *source_path, char *target_path, GError **error)
{
    if (*error != NULL) return FALSE;

    GQuark quark;
    quark = g_quark_from_string (GW_IO_ERROR);

    char *contents = NULL;
    gsize length;
    if ( g_file_get_contents(source_path, &contents, &length, NULL) == FALSE ||
         g_file_set_contents(target_path, contents, length, NULL) == FALSE     )
    {
      remove(target_path);
      if (error != NULL) *error = g_error_new_literal (quark, GW_IO_COPY_ERROR, gettext("File copy error"));
      return FALSE;
    }
    if (contents != NULL) g_free(contents);

    return TRUE;
}


//Creates a single dictionary containing both the radical dict and kanji dict
gboolean gw_io_create_mix_dictionary (char *mpath, char *kpath, char *rpath)
{
    FILE* kanji_file =  fopen(kpath, "r");
    FILE* output_file = fopen(mpath, "w");
    FILE *radicals_file = NULL;

    char radicals_input[GW_IO_MAX_FGETS_LINE];
    char* radicals_ptr = NULL;

    char kanji_input[GW_IO_MAX_FGETS_LINE];
    char* kanji_ptr = NULL;

    char output[GW_IO_MAX_FGETS_LINE * 2];
    char* output_ptr = NULL;

    char* temp_ptr;

    //Loop through the kanji file
    while ( fgets(kanji_input, GW_IO_MAX_FGETS_LINE, kanji_file) != NULL )
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
      while ( fgets(radicals_input, GW_IO_MAX_FGETS_LINE, radicals_file) != NULL )
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
gboolean gw_io_gunzip_file (char *path, GError **error)
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
gboolean gw_io_unzip_file (char *path, GError **error)
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
//! \brief Uninstalls a dictionary by it's name and all related dictionaries
//!
//! @param name The string representing the name of the dictionary to uninstall.
//! @param callback_function The function to use to post status messages to.
//! @param data gpointer to the data to pass to the callback_function
//! @param long_messages Whether the messages should be long form or short form.
//!
void gw_io_uninstall_dictinfo (GwDictInfo *di,    int (*callback_function) (char*, int, gpointer),
                                 gpointer data, gboolean long_messages                            )
{
  /*
    if (di == NULL) return;

    char *path = g_build_filename (gw_util_get_directory_for_engine (di->engine), di->name);

    char *message = NULL;
    if (long_messages)
      // TRANSLATORS: The %s stands for a file path
      message = g_strdup_printf (gettext("Removing %s..."), path);
    else
      message = g_strdup_printf (gettext("Removing..."));

    if (callback_function != NULL) {
      callback_function (message, -1, data);
    }

    g_free (message);
    message = NULL;

    g_remove(path);

    g_free (path);
    path = NULL;

    di->status = GW_DICT_STATUS_NOT_INSTALLED;
    di->load_position = -1;
    */
}


//!
//! @brief Installs a dictionary by it's name and all related dictionaries
//!
//! @param name The string representing the name of the dictionary to uninstall.
//! @param callback_function The function to use to post status messages to.
//! @param data gpointer to the data to pass to the callback_function
//! @param long_messages Whether the messages should be long form or short form.
//!
void gw_io_install_dictinfo (GwDictInfo *di,    int (*callback_function) (char*, int, gpointer),
                               gpointer data, gboolean long_messages, GError **error            )
{
/*
    if (*error != NULL) return;

    GQuark quark;
    quark = g_quark_from_string (GW_IO_ERROR);

    if (di == NULL || di->status != GW_DICT_STATUS_NOT_INSTALLED) return;
    di->status = GW_DICT_STATUS_INSTALLING;

    char *message = NULL;

    g_assert (di->source_uri != NULL);
    char uri[100];
    strncpy(uri, di->source_uri, 100);

    if (long_messages == TRUE)
    {
      // TRANSLATORS: The %s stands for an URL
      message = g_strdup_printf(gettext("Downloading %s..."), uri);
      if (message != NULL)
      {
        callback_function (message, -1, NULL);
        g_free (message);
        message = NULL;
      }
    }

    //Set up the path to save to
    char download_name[strlen(di->name + 30)];
    switch (di->encoding)
    {
      case GW_ENCODING_EUC_JP:
        strcat(download_name, ".EUC-JP");
        break;
      case GW_ENCODING_SHIFT_JS:
        strcat(download_name, ".SHIFT_JS");
        break;
      case GW_ENCODING_UTF8:
        strcat(download_name, ".UTF8");
        break;
    }
    char *encoding_path = g_build_filename (gw_util_get_directory(GW_PATH_CACHE), download_name, NULL);

    switch (di->engine)
    {
      case GW_DICT_COMPRESSION_ZIP:
        strcat(download_name, ".zip");
        break;
      case GW_DICT_COMPRESSION_GZIP:
        strcat(download_name, ".gz");
        break;
      default:
        strcat(download_name, ".uncompressed");
        break;
    }
    char *download_path = g_build_filename (gw_util_get_directory(GW_PATH_CACHE), download_name, NULL);
    char *final_path = g_build_filename (gw_util_get_directory_for_engine(di->engine), di->name, NULL);


    //Copy the file if it is a local file
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
    {
      if (g_file_test (uri, G_FILE_TEST_IS_REGULAR)) //Copy from local drive
      {
        gw_io_copy_file (uri, download_path, error);
      }
      else //Download file over network
      {
        gw_io_download_file (uri, download_path, callback_function, data, error);
      }
    }

    //Decompress the file if necessary
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING && di->compression == GW_DICT_COMPRESSION_GZIP)
    {
      if (callback_function != NULL) {
        callback_function (gettext("Decompressing..."), -1, data);
      }
      gw_io_gunzip_file (download_path, error);
    }
    else if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING && di->compression == GW_DICT_COMPRESSION_ZIP)
    {
      gw_io_unzip_file(download_path, error);
      if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
      {
        quark = g_quark_from_string (GW_IO_ERROR);
        *error = g_error_new_literal (quark, GW_IO_DECOMPRESSION_ERROR, gettext("Unzip Error"));
      }
    }
    else
    {
      g_rename (download_path, encoding_path);
    }

    //Convert encoding if necessary
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING && di->encoding != GW_ENCODING_UTF8)
    {
      if (callback_function != NULL) {
        callback_function (gettext("Converting encoding..."), -1, data);
      }
      printf("converting encoding...\n");
      char source_encoding[100];
      if (di->encoding == GW_ENCODING_EUC_JP) strcpy(source_encoding, "EUC-JP");
      if (di->encoding == GW_ENCODING_SHIFT_JS) strcpy(source_encoding, "Shift-JS");
      gw_io_copy_with_encoding (encoding_path, final_path, "EUC-JP","UTF-8", error);
    }
    else if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
    {
      gw_io_copy_file (encoding_path, final_path, error);
    }

    //Special dictionary post processing
    if (*error == NULL && di->status != GW_DICT_STATUS_CANCELING)
    {
      if (callback_function != NULL) {
        callback_function (gettext("Postprocessing..."), -1, data);
      }
      gw_dictlist_preform_postprocessing_by_name (di->name, error);
    }

    g_free (encoding_path);
    g_free (download_path);
    g_free (final_path);
*/
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
