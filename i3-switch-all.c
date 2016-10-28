#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <json-glib/json-glib.h>

int main ()
{
  gchar *reply;
  JsonParser* parser;
  JsonReader* reader;
  JsonNode* root;
  int workspaces, i, j, visible;
  const gchar** names;

  g_spawn_command_line_sync("i3-msg -t get_workspaces", &reply, NULL, NULL, NULL);

  parser = json_parser_new();
  if (!json_parser_load_from_data(parser, reply, -1, NULL))
  {
    exit(1);
  }

  root = json_parser_get_root(parser);
  reader = json_reader_new(root);
  workspaces = json_reader_count_elements(reader);
  names = g_malloc(sizeof(gchar*) * workspaces);
  j = 0;

  for (i = 0; i < workspaces; i++)
  {
    json_reader_read_element(reader, i);
    json_reader_read_member(reader, "visible");
    visible = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    if (!visible) {
      json_reader_read_member(reader, "name");
      names[j] = json_reader_get_string_value(reader);
      json_reader_end_member(reader);
      ++j;
    }
    json_reader_end_element(reader);
  }

  for (i = 0; i < workspaces; i++)
  {
    json_reader_read_element(reader, i);
    json_reader_read_member(reader, "visible");
    visible = json_reader_get_boolean_value(reader);
    json_reader_end_member(reader);
    if (visible) {
      json_reader_read_member(reader, "name");
      names[j] = json_reader_get_string_value(reader);
      json_reader_end_member(reader);
      ++j;
    }
    json_reader_end_element(reader);
  }

  GString* command = g_string_new("i3-msg ");

  g_string_append(command, "mark switch-all; ");

  for (i = 0; i < workspaces; i++)
  {
    g_string_append_printf(command, "workspace %s; ", names[i]);
    g_string_append(command, "move workspace to output left; ");
  }

  g_string_append(command, "[con_mark=\"switch-all\"] focus, ");
  g_string_append(command, "unmark switch-all; ");

  g_printf("%s\n", command->str);
  g_spawn_command_line_sync(command->str, NULL, NULL, NULL, NULL);

  g_string_free(command, TRUE);
  g_free(reply);
  g_object_unref(parser);
  g_object_unref(reader);
  g_free(names);

  return EXIT_SUCCESS;
}
