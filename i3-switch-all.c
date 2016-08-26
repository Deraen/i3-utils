#include <stdlib.h>
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
  gchar command[100];

  g_spawn_command_line_sync("i3-msg -t get_workspaces", &reply, NULL, NULL, NULL);

  parser = json_parser_new();
  if (!json_parser_load_from_data(parser, reply, -1, NULL))
  {
    exit(1);
  }

  root = json_parser_get_root(parser);
  reader = json_reader_new(root);

  g_spawn_command_line_sync("3-msg mark switch-all", NULL, NULL, NULL, NULL);

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

  for (i = 0; i < workspaces; i++)
  {
    g_sprintf(command, "i3-msg workspace %s", names[i]);
    g_spawn_command_line_sync(command, NULL, NULL, NULL, NULL);
    g_spawn_command_line_sync("i3-msg move workspace to output left", NULL, NULL, NULL, NULL);
  }

  g_spawn_command_line_sync("3-msg [con_mark=\"switch-all\"] focus", NULL, NULL, NULL, NULL);
  g_spawn_command_line_sync("3-msg unmark switch-all", NULL, NULL, NULL, NULL);

  g_free(reply);
  g_object_unref(parser);
  g_object_unref(reader);
  g_free(names);

  return EXIT_SUCCESS;
}
