/*
  File autogenerated by gengetopt version 2.22.4
  generated with the following command:
  gengetopt -a speed_knn_args -F speed_knn_args -i speed_knn.ggo 

  The developers of gengetopt consider the fixed text that goes in all
  gengetopt output files to be in the public domain:
  we make no copyright claims on it.
*/

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef FIX_UNUSED
#define FIX_UNUSED(X) (void) (X) /* avoid warnings for unused params */
#endif

#include <getopt.h>

#include "speed_knn_args.h"

const char *speed_knn_args_purpose = "Measure the speed of the K nearest neighbours operation with provided or \ngenerated data.";

const char *speed_knn_args_usage = "Usage: -K <# neighbours> <train set input> <test set input> [other options]";

const char *speed_knn_args_description = "";

const char *speed_knn_args_help[] = {
  "  -h, --help                    Print help and exit",
  "  -V, --version                 Print version and exit",
  "  -k, --knn=INT                 Search for the K nearest neighbours. Set to 0 \n                                  to disable.  (default=`10')",
  "\nRequired params:",
  "\n Group: Train set input",
  "  -F, --train-file=STRING       Read the train set from a file.",
  "  -T, --train-random=INT        Generate a random train set of the specified \n                                  size.",
  "      --train-save-random=STRING\n                                Save the randomly generated train set to the \n                                  specified file, if provided.",
  "\n Group: Test set input",
  "  -f, --test-file=STRING        Read the test set from a file.",
  "  -t, --test-random=INT         Generate a random test set of the specified \n                                  size.",
  "      --test-save-random=STRING Save the randomly generated test set to the \n                                  specified file, if provided.",
  "\nOther options:",
  "  -l, --tolerance=FLOAT         Tolerance value used when comparing the \n                                  distance values to the exhaustive search \n                                  version.  (default=`1e-3')",
  "  -b, --bucket-size=INT         Size of the buckets containing elements at the \n                                  leaf nodes of the kd-tree.  (default=`32')",
  "  -x, --test-from-train=FLOAT   Set the % probability of test entries to be \n                                  random elements from the train set.  \n                                  (default=`20')",
  "  -i, --ignore-existing         Ignore any existing instances in the tree of \n                                  the vector being tested.  (default=off)",
  "  -e, --epsilon=FLOAT           Distance added to the intersection calculations \n                                  to approximate the results by rejecting more \n                                  candidates. May raise result errors.  \n                                  (default=`0')",
  "  -R, --random-range=FLOAT      Set the range of the numbers to be randomly \n                                  generated (from 0 to the specified value).  \n                                  (default=`100')",
  "  -s, --random-seed=INT         Seed used to initialize random number \n                                  generator. Allows repeatable tests.",
    0
};

typedef enum {ARG_NO
  , ARG_FLAG
  , ARG_STRING
  , ARG_INT
  , ARG_FLOAT
} cmdline_parser_arg_type;

static
void clear_given (struct speed_knn_args *args_info);
static
void clear_args (struct speed_knn_args *args_info);

static int
cmdline_parser_internal (int argc, char **argv, struct speed_knn_args *args_info,
                        struct cmdline_parser_params *params, const char *additional_error);

static int
cmdline_parser_required2 (struct speed_knn_args *args_info, const char *prog_name, const char *additional_error);

static char *
gengetopt_strdup (const char *s);

static
void clear_given (struct speed_knn_args *args_info)
{
  args_info->help_given = 0 ;
  args_info->version_given = 0 ;
  args_info->knn_given = 0 ;
  args_info->train_file_given = 0 ;
  args_info->train_random_given = 0 ;
  args_info->train_save_random_given = 0 ;
  args_info->test_file_given = 0 ;
  args_info->test_random_given = 0 ;
  args_info->test_save_random_given = 0 ;
  args_info->tolerance_given = 0 ;
  args_info->bucket_size_given = 0 ;
  args_info->test_from_train_given = 0 ;
  args_info->ignore_existing_given = 0 ;
  args_info->epsilon_given = 0 ;
  args_info->random_range_given = 0 ;
  args_info->random_seed_given = 0 ;
  args_info->Test_set_input_group_counter = 0 ;
  args_info->Train_set_input_group_counter = 0 ;
}

static
void clear_args (struct speed_knn_args *args_info)
{
  FIX_UNUSED (args_info);
  args_info->knn_arg = 10;
  args_info->knn_orig = NULL;
  args_info->train_file_arg = NULL;
  args_info->train_file_orig = NULL;
  args_info->train_random_orig = NULL;
  args_info->train_save_random_arg = NULL;
  args_info->train_save_random_orig = NULL;
  args_info->test_file_arg = NULL;
  args_info->test_file_orig = NULL;
  args_info->test_random_orig = NULL;
  args_info->test_save_random_arg = NULL;
  args_info->test_save_random_orig = NULL;
  args_info->tolerance_arg = 1e-3;
  args_info->tolerance_orig = NULL;
  args_info->bucket_size_arg = 32;
  args_info->bucket_size_orig = NULL;
  args_info->test_from_train_arg = 20;
  args_info->test_from_train_orig = NULL;
  args_info->ignore_existing_flag = 0;
  args_info->epsilon_arg = 0;
  args_info->epsilon_orig = NULL;
  args_info->random_range_arg = 100;
  args_info->random_range_orig = NULL;
  args_info->random_seed_orig = NULL;
  
}

static
void init_args_info(struct speed_knn_args *args_info)
{


  args_info->help_help = speed_knn_args_help[0] ;
  args_info->version_help = speed_knn_args_help[1] ;
  args_info->knn_help = speed_knn_args_help[2] ;
  args_info->train_file_help = speed_knn_args_help[5] ;
  args_info->train_random_help = speed_knn_args_help[6] ;
  args_info->train_save_random_help = speed_knn_args_help[7] ;
  args_info->test_file_help = speed_knn_args_help[9] ;
  args_info->test_random_help = speed_knn_args_help[10] ;
  args_info->test_save_random_help = speed_knn_args_help[11] ;
  args_info->tolerance_help = speed_knn_args_help[13] ;
  args_info->bucket_size_help = speed_knn_args_help[14] ;
  args_info->test_from_train_help = speed_knn_args_help[15] ;
  args_info->ignore_existing_help = speed_knn_args_help[16] ;
  args_info->epsilon_help = speed_knn_args_help[17] ;
  args_info->random_range_help = speed_knn_args_help[18] ;
  args_info->random_seed_help = speed_knn_args_help[19] ;
  
}

void
cmdline_parser_print_version (void)
{
  printf ("%s %s\n",
     (strlen(CMDLINE_PARSER_PACKAGE_NAME) ? CMDLINE_PARSER_PACKAGE_NAME : CMDLINE_PARSER_PACKAGE),
     CMDLINE_PARSER_VERSION);
}

static void print_help_common(void) {
  cmdline_parser_print_version ();

  if (strlen(speed_knn_args_purpose) > 0)
    printf("\n%s\n", speed_knn_args_purpose);

  if (strlen(speed_knn_args_usage) > 0)
    printf("\n%s\n", speed_knn_args_usage);

  printf("\n");

  if (strlen(speed_knn_args_description) > 0)
    printf("%s\n\n", speed_knn_args_description);
}

void
cmdline_parser_print_help (void)
{
  int i = 0;
  print_help_common();
  while (speed_knn_args_help[i])
    printf("%s\n", speed_knn_args_help[i++]);
}

void
cmdline_parser_init (struct speed_knn_args *args_info)
{
  clear_given (args_info);
  clear_args (args_info);
  init_args_info (args_info);
}

void
cmdline_parser_params_init(struct cmdline_parser_params *params)
{
  if (params)
    { 
      params->override = 0;
      params->initialize = 1;
      params->check_required = 1;
      params->check_ambiguity = 0;
      params->print_errors = 1;
    }
}

struct cmdline_parser_params *
cmdline_parser_params_create(void)
{
  struct cmdline_parser_params *params = 
    (struct cmdline_parser_params *)malloc(sizeof(struct cmdline_parser_params));
  cmdline_parser_params_init(params);  
  return params;
}

static void
free_string_field (char **s)
{
  if (*s)
    {
      free (*s);
      *s = 0;
    }
}


static void
cmdline_parser_release (struct speed_knn_args *args_info)
{

  free_string_field (&(args_info->knn_orig));
  free_string_field (&(args_info->train_file_arg));
  free_string_field (&(args_info->train_file_orig));
  free_string_field (&(args_info->train_random_orig));
  free_string_field (&(args_info->train_save_random_arg));
  free_string_field (&(args_info->train_save_random_orig));
  free_string_field (&(args_info->test_file_arg));
  free_string_field (&(args_info->test_file_orig));
  free_string_field (&(args_info->test_random_orig));
  free_string_field (&(args_info->test_save_random_arg));
  free_string_field (&(args_info->test_save_random_orig));
  free_string_field (&(args_info->tolerance_orig));
  free_string_field (&(args_info->bucket_size_orig));
  free_string_field (&(args_info->test_from_train_orig));
  free_string_field (&(args_info->epsilon_orig));
  free_string_field (&(args_info->random_range_orig));
  free_string_field (&(args_info->random_seed_orig));
  
  

  clear_given (args_info);
}


static void
write_into_file(FILE *outfile, const char *opt, const char *arg, const char *values[])
{
  FIX_UNUSED (values);
  if (arg) {
    fprintf(outfile, "%s=\"%s\"\n", opt, arg);
  } else {
    fprintf(outfile, "%s\n", opt);
  }
}


int
cmdline_parser_dump(FILE *outfile, struct speed_knn_args *args_info)
{
  int i = 0;

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot dump options to stream\n", CMDLINE_PARSER_PACKAGE);
      return EXIT_FAILURE;
    }

  if (args_info->help_given)
    write_into_file(outfile, "help", 0, 0 );
  if (args_info->version_given)
    write_into_file(outfile, "version", 0, 0 );
  if (args_info->knn_given)
    write_into_file(outfile, "knn", args_info->knn_orig, 0);
  if (args_info->train_file_given)
    write_into_file(outfile, "train-file", args_info->train_file_orig, 0);
  if (args_info->train_random_given)
    write_into_file(outfile, "train-random", args_info->train_random_orig, 0);
  if (args_info->train_save_random_given)
    write_into_file(outfile, "train-save-random", args_info->train_save_random_orig, 0);
  if (args_info->test_file_given)
    write_into_file(outfile, "test-file", args_info->test_file_orig, 0);
  if (args_info->test_random_given)
    write_into_file(outfile, "test-random", args_info->test_random_orig, 0);
  if (args_info->test_save_random_given)
    write_into_file(outfile, "test-save-random", args_info->test_save_random_orig, 0);
  if (args_info->tolerance_given)
    write_into_file(outfile, "tolerance", args_info->tolerance_orig, 0);
  if (args_info->bucket_size_given)
    write_into_file(outfile, "bucket-size", args_info->bucket_size_orig, 0);
  if (args_info->test_from_train_given)
    write_into_file(outfile, "test-from-train", args_info->test_from_train_orig, 0);
  if (args_info->ignore_existing_given)
    write_into_file(outfile, "ignore-existing", 0, 0 );
  if (args_info->epsilon_given)
    write_into_file(outfile, "epsilon", args_info->epsilon_orig, 0);
  if (args_info->random_range_given)
    write_into_file(outfile, "random-range", args_info->random_range_orig, 0);
  if (args_info->random_seed_given)
    write_into_file(outfile, "random-seed", args_info->random_seed_orig, 0);
  

  i = EXIT_SUCCESS;
  return i;
}

int
cmdline_parser_file_save(const char *filename, struct speed_knn_args *args_info)
{
  FILE *outfile;
  int i = 0;

  outfile = fopen(filename, "w");

  if (!outfile)
    {
      fprintf (stderr, "%s: cannot open file for writing: %s\n", CMDLINE_PARSER_PACKAGE, filename);
      return EXIT_FAILURE;
    }

  i = cmdline_parser_dump(outfile, args_info);
  fclose (outfile);

  return i;
}

void
cmdline_parser_free (struct speed_knn_args *args_info)
{
  cmdline_parser_release (args_info);
}

/** @brief replacement of strdup, which is not standard */
char *
gengetopt_strdup (const char *s)
{
  char *result = 0;
  if (!s)
    return result;

  result = (char*)malloc(strlen(s) + 1);
  if (result == (char*)0)
    return (char*)0;
  strcpy(result, s);
  return result;
}

static void
reset_group_Test_set_input(struct speed_knn_args *args_info)
{
  if (! args_info->Test_set_input_group_counter)
    return;
  
  args_info->test_file_given = 0 ;
  free_string_field (&(args_info->test_file_arg));
  free_string_field (&(args_info->test_file_orig));
  args_info->test_random_given = 0 ;
  free_string_field (&(args_info->test_random_orig));

  args_info->Test_set_input_group_counter = 0;
}

static void
reset_group_Train_set_input(struct speed_knn_args *args_info)
{
  if (! args_info->Train_set_input_group_counter)
    return;
  
  args_info->train_file_given = 0 ;
  free_string_field (&(args_info->train_file_arg));
  free_string_field (&(args_info->train_file_orig));
  args_info->train_random_given = 0 ;
  free_string_field (&(args_info->train_random_orig));

  args_info->Train_set_input_group_counter = 0;
}

int
cmdline_parser (int argc, char **argv, struct speed_knn_args *args_info)
{
  return cmdline_parser2 (argc, argv, args_info, 0, 1, 1);
}

int
cmdline_parser_ext (int argc, char **argv, struct speed_knn_args *args_info,
                   struct cmdline_parser_params *params)
{
  int result;
  result = cmdline_parser_internal (argc, argv, args_info, params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser2 (int argc, char **argv, struct speed_knn_args *args_info, int override, int initialize, int check_required)
{
  int result;
  struct cmdline_parser_params params;
  
  params.override = override;
  params.initialize = initialize;
  params.check_required = check_required;
  params.check_ambiguity = 0;
  params.print_errors = 1;

  result = cmdline_parser_internal (argc, argv, args_info, &params, 0);

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser_required (struct speed_knn_args *args_info, const char *prog_name)
{
  int result = EXIT_SUCCESS;

  if (cmdline_parser_required2(args_info, prog_name, 0) > 0)
    result = EXIT_FAILURE;

  if (result == EXIT_FAILURE)
    {
      cmdline_parser_free (args_info);
      exit (EXIT_FAILURE);
    }
  
  return result;
}

int
cmdline_parser_required2 (struct speed_knn_args *args_info, const char *prog_name, const char *additional_error)
{
  int error = 0;
  FIX_UNUSED (additional_error);

  /* checks for required options */
  if (! args_info->knn_given)
    {
      fprintf (stderr, "%s: '--knn' ('-k') option required%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }
  
  if (args_info->Test_set_input_group_counter == 0)
    {
      fprintf (stderr, "%s: %d options of group Test set input were given. One is required%s.\n", prog_name, args_info->Test_set_input_group_counter, (additional_error ? additional_error : ""));
      error = 1;
    }
  
if (args_info->Train_set_input_group_counter == 0)
    {
      fprintf (stderr, "%s: %d options of group Train set input were given. One is required%s.\n", prog_name, args_info->Train_set_input_group_counter, (additional_error ? additional_error : ""));
      error = 1;
    }
  

  /* checks for dependences among options */
  if (args_info->train_save_random_given && ! args_info->train_random_given)
    {
      fprintf (stderr, "%s: '--train-save-random' option depends on option 'train-random'%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }
  if (args_info->test_save_random_given && ! args_info->test_random_given)
    {
      fprintf (stderr, "%s: '--test-save-random' option depends on option 'test-random'%s\n", prog_name, (additional_error ? additional_error : ""));
      error = 1;
    }

  return error;
}


static char *package_name = 0;

/**
 * @brief updates an option
 * @param field the generic pointer to the field to update
 * @param orig_field the pointer to the orig field
 * @param field_given the pointer to the number of occurrence of this option
 * @param prev_given the pointer to the number of occurrence already seen
 * @param value the argument for this option (if null no arg was specified)
 * @param possible_values the possible values for this option (if specified)
 * @param default_value the default value (in case the option only accepts fixed values)
 * @param arg_type the type of this option
 * @param check_ambiguity @see cmdline_parser_params.check_ambiguity
 * @param override @see cmdline_parser_params.override
 * @param no_free whether to free a possible previous value
 * @param multiple_option whether this is a multiple option
 * @param long_opt the corresponding long option
 * @param short_opt the corresponding short option (or '-' if none)
 * @param additional_error possible further error specification
 */
static
int update_arg(void *field, char **orig_field,
               unsigned int *field_given, unsigned int *prev_given, 
               char *value, const char *possible_values[],
               const char *default_value,
               cmdline_parser_arg_type arg_type,
               int check_ambiguity, int override,
               int no_free, int multiple_option,
               const char *long_opt, char short_opt,
               const char *additional_error)
{
  char *stop_char = 0;
  const char *val = value;
  int found;
  char **string_field;
  FIX_UNUSED (field);

  stop_char = 0;
  found = 0;

  if (!multiple_option && prev_given && (*prev_given || (check_ambiguity && *field_given)))
    {
      if (short_opt != '-')
        fprintf (stderr, "%s: `--%s' (`-%c') option given more than once%s\n", 
               package_name, long_opt, short_opt,
               (additional_error ? additional_error : ""));
      else
        fprintf (stderr, "%s: `--%s' option given more than once%s\n", 
               package_name, long_opt,
               (additional_error ? additional_error : ""));
      return 1; /* failure */
    }

  FIX_UNUSED (default_value);
    
  if (field_given && *field_given && ! override)
    return 0;
  if (prev_given)
    (*prev_given)++;
  if (field_given)
    (*field_given)++;
  if (possible_values)
    val = possible_values[found];

  switch(arg_type) {
  case ARG_FLAG:
    *((int *)field) = !*((int *)field);
    break;
  case ARG_INT:
    if (val) *((int *)field) = strtol (val, &stop_char, 0);
    break;
  case ARG_FLOAT:
    if (val) *((float *)field) = (float)strtod (val, &stop_char);
    break;
  case ARG_STRING:
    if (val) {
      string_field = (char **)field;
      if (!no_free && *string_field)
        free (*string_field); /* free previous string */
      *string_field = gengetopt_strdup (val);
    }
    break;
  default:
    break;
  };

  /* check numeric conversion */
  switch(arg_type) {
  case ARG_INT:
  case ARG_FLOAT:
    if (val && !(stop_char && *stop_char == '\0')) {
      fprintf(stderr, "%s: invalid numeric value: %s\n", package_name, val);
      return 1; /* failure */
    }
    break;
  default:
    ;
  };

  /* store the original value */
  switch(arg_type) {
  case ARG_NO:
  case ARG_FLAG:
    break;
  default:
    if (value && orig_field) {
      if (no_free) {
        *orig_field = value;
      } else {
        if (*orig_field)
          free (*orig_field); /* free previous string */
        *orig_field = gengetopt_strdup (value);
      }
    }
  };

  return 0; /* OK */
}


int
cmdline_parser_internal (
  int argc, char **argv, struct speed_knn_args *args_info,
                        struct cmdline_parser_params *params, const char *additional_error)
{
  int c;	/* Character of the parsed option.  */

  int error = 0;
  struct speed_knn_args local_args_info;
  
  int override;
  int initialize;
  int check_required;
  int check_ambiguity;
  
  package_name = argv[0];
  
  override = params->override;
  initialize = params->initialize;
  check_required = params->check_required;
  check_ambiguity = params->check_ambiguity;

  if (initialize)
    cmdline_parser_init (args_info);

  cmdline_parser_init (&local_args_info);

  optarg = 0;
  optind = 0;
  opterr = params->print_errors;
  optopt = '?';

  while (1)
    {
      int option_index = 0;

      static struct option long_options[] = {
        { "help",	0, NULL, 'h' },
        { "version",	0, NULL, 'V' },
        { "knn",	1, NULL, 'k' },
        { "train-file",	1, NULL, 'F' },
        { "train-random",	1, NULL, 'T' },
        { "train-save-random",	1, NULL, 0 },
        { "test-file",	1, NULL, 'f' },
        { "test-random",	1, NULL, 't' },
        { "test-save-random",	1, NULL, 0 },
        { "tolerance",	1, NULL, 'l' },
        { "bucket-size",	1, NULL, 'b' },
        { "test-from-train",	1, NULL, 'x' },
        { "ignore-existing",	0, NULL, 'i' },
        { "epsilon",	1, NULL, 'e' },
        { "random-range",	1, NULL, 'R' },
        { "random-seed",	1, NULL, 's' },
        { 0,  0, 0, 0 }
      };

      c = getopt_long (argc, argv, "hVk:F:T:f:t:l:b:x:ie:R:s:", long_options, &option_index);

      if (c == -1) break;	/* Exit from `while (1)' loop.  */

      switch (c)
        {
        case 'h':	/* Print help and exit.  */
          cmdline_parser_print_help ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'V':	/* Print version and exit.  */
          cmdline_parser_print_version ();
          cmdline_parser_free (&local_args_info);
          exit (EXIT_SUCCESS);

        case 'k':	/* Search for the K nearest neighbours. Set to 0 to disable..  */
        
        
          if (update_arg( (void *)&(args_info->knn_arg), 
               &(args_info->knn_orig), &(args_info->knn_given),
              &(local_args_info.knn_given), optarg, 0, "10", ARG_INT,
              check_ambiguity, override, 0, 0,
              "knn", 'k',
              additional_error))
            goto failure;
        
          break;
        case 'F':	/* Read the train set from a file..  */
        
          if (args_info->Train_set_input_group_counter && override)
            reset_group_Train_set_input (args_info);
          args_info->Train_set_input_group_counter += 1;
        
          if (update_arg( (void *)&(args_info->train_file_arg), 
               &(args_info->train_file_orig), &(args_info->train_file_given),
              &(local_args_info.train_file_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "train-file", 'F',
              additional_error))
            goto failure;
        
          break;
        case 'T':	/* Generate a random train set of the specified size..  */
        
          if (args_info->Train_set_input_group_counter && override)
            reset_group_Train_set_input (args_info);
          args_info->Train_set_input_group_counter += 1;
        
          if (update_arg( (void *)&(args_info->train_random_arg), 
               &(args_info->train_random_orig), &(args_info->train_random_given),
              &(local_args_info.train_random_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "train-random", 'T',
              additional_error))
            goto failure;
        
          break;
        case 'f':	/* Read the test set from a file..  */
        
          if (args_info->Test_set_input_group_counter && override)
            reset_group_Test_set_input (args_info);
          args_info->Test_set_input_group_counter += 1;
        
          if (update_arg( (void *)&(args_info->test_file_arg), 
               &(args_info->test_file_orig), &(args_info->test_file_given),
              &(local_args_info.test_file_given), optarg, 0, 0, ARG_STRING,
              check_ambiguity, override, 0, 0,
              "test-file", 'f',
              additional_error))
            goto failure;
        
          break;
        case 't':	/* Generate a random test set of the specified size..  */
        
          if (args_info->Test_set_input_group_counter && override)
            reset_group_Test_set_input (args_info);
          args_info->Test_set_input_group_counter += 1;
        
          if (update_arg( (void *)&(args_info->test_random_arg), 
               &(args_info->test_random_orig), &(args_info->test_random_given),
              &(local_args_info.test_random_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "test-random", 't',
              additional_error))
            goto failure;
        
          break;
        case 'l':	/* Tolerance value used when comparing the distance values to the exhaustive search version..  */
        
        
          if (update_arg( (void *)&(args_info->tolerance_arg), 
               &(args_info->tolerance_orig), &(args_info->tolerance_given),
              &(local_args_info.tolerance_given), optarg, 0, "1e-3", ARG_FLOAT,
              check_ambiguity, override, 0, 0,
              "tolerance", 'l',
              additional_error))
            goto failure;
        
          break;
        case 'b':	/* Size of the buckets containing elements at the leaf nodes of the kd-tree..  */
        
        
          if (update_arg( (void *)&(args_info->bucket_size_arg), 
               &(args_info->bucket_size_orig), &(args_info->bucket_size_given),
              &(local_args_info.bucket_size_given), optarg, 0, "32", ARG_INT,
              check_ambiguity, override, 0, 0,
              "bucket-size", 'b',
              additional_error))
            goto failure;
        
          break;
        case 'x':	/* Set the % probability of test entries to be random elements from the train set..  */
        
        
          if (update_arg( (void *)&(args_info->test_from_train_arg), 
               &(args_info->test_from_train_orig), &(args_info->test_from_train_given),
              &(local_args_info.test_from_train_given), optarg, 0, "20", ARG_FLOAT,
              check_ambiguity, override, 0, 0,
              "test-from-train", 'x',
              additional_error))
            goto failure;
        
          break;
        case 'i':	/* Ignore any existing instances in the tree of the vector being tested..  */
        
        
          if (update_arg((void *)&(args_info->ignore_existing_flag), 0, &(args_info->ignore_existing_given),
              &(local_args_info.ignore_existing_given), optarg, 0, 0, ARG_FLAG,
              check_ambiguity, override, 1, 0, "ignore-existing", 'i',
              additional_error))
            goto failure;
        
          break;
        case 'e':	/* Distance added to the intersection calculations to approximate the results by rejecting more candidates. May raise result errors..  */
        
        
          if (update_arg( (void *)&(args_info->epsilon_arg), 
               &(args_info->epsilon_orig), &(args_info->epsilon_given),
              &(local_args_info.epsilon_given), optarg, 0, "0", ARG_FLOAT,
              check_ambiguity, override, 0, 0,
              "epsilon", 'e',
              additional_error))
            goto failure;
        
          break;
        case 'R':	/* Set the range of the numbers to be randomly generated (from 0 to the specified value)..  */
        
        
          if (update_arg( (void *)&(args_info->random_range_arg), 
               &(args_info->random_range_orig), &(args_info->random_range_given),
              &(local_args_info.random_range_given), optarg, 0, "100", ARG_FLOAT,
              check_ambiguity, override, 0, 0,
              "random-range", 'R',
              additional_error))
            goto failure;
        
          break;
        case 's':	/* Seed used to initialize random number generator. Allows repeatable tests..  */
        
        
          if (update_arg( (void *)&(args_info->random_seed_arg), 
               &(args_info->random_seed_orig), &(args_info->random_seed_given),
              &(local_args_info.random_seed_given), optarg, 0, 0, ARG_INT,
              check_ambiguity, override, 0, 0,
              "random-seed", 's',
              additional_error))
            goto failure;
        
          break;

        case 0:	/* Long option with no short option */
          /* Save the randomly generated train set to the specified file, if provided..  */
          if (strcmp (long_options[option_index].name, "train-save-random") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->train_save_random_arg), 
                 &(args_info->train_save_random_orig), &(args_info->train_save_random_given),
                &(local_args_info.train_save_random_given), optarg, 0, 0, ARG_STRING,
                check_ambiguity, override, 0, 0,
                "train-save-random", '-',
                additional_error))
              goto failure;
          
          }
          /* Save the randomly generated test set to the specified file, if provided..  */
          else if (strcmp (long_options[option_index].name, "test-save-random") == 0)
          {
          
          
            if (update_arg( (void *)&(args_info->test_save_random_arg), 
                 &(args_info->test_save_random_orig), &(args_info->test_save_random_given),
                &(local_args_info.test_save_random_given), optarg, 0, 0, ARG_STRING,
                check_ambiguity, override, 0, 0,
                "test-save-random", '-',
                additional_error))
              goto failure;
          
          }
          
          break;
        case '?':	/* Invalid option.  */
          /* `getopt_long' already printed an error message.  */
          goto failure;

        default:	/* bug: option not considered.  */
          fprintf (stderr, "%s: option unknown: %c%s\n", CMDLINE_PARSER_PACKAGE, c, (additional_error ? additional_error : ""));
          abort ();
        } /* switch */
    } /* while */

  if (args_info->Test_set_input_group_counter > 1)
    {
      fprintf (stderr, "%s: %d options of group Test set input were given. One is required%s.\n", argv[0], args_info->Test_set_input_group_counter, (additional_error ? additional_error : ""));
      error = 1;
    }
  
  if (args_info->Train_set_input_group_counter > 1)
    {
      fprintf (stderr, "%s: %d options of group Train set input were given. One is required%s.\n", argv[0], args_info->Train_set_input_group_counter, (additional_error ? additional_error : ""));
      error = 1;
    }
  


  if (check_required)
    {
      error += cmdline_parser_required2 (args_info, argv[0], additional_error);
    }

  cmdline_parser_release (&local_args_info);

  if ( error )
    return (EXIT_FAILURE);

  return 0;

failure:
  
  cmdline_parser_release (&local_args_info);
  return (EXIT_FAILURE);
}
