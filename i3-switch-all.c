#include <stdlib.h>
#include <glib/gprintf.h>
#include <i3ipc-glib/i3ipc-glib.h>
#include <json-glib/json-glib.h>

int main ()
{
  i3ipcConnection *conn;
  gchar *reply;
  JsonParser* parser;
  JsonReader* reader;
  JsonNode* root;
  int workspaces, i, j, visible;
  const gchar** names;
  gchar command[100];

  conn = i3ipc_connection_new(NULL, NULL);
  reply = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_GET_WORKSPACES, NULL, NULL);

  parser = json_parser_new();
  if (!json_parser_load_from_data(parser, reply, -1, NULL))
  {
    exit(1);
  }

  root = json_parser_get_root(parser);
  reader = json_reader_new(root);

  i3ipc_connection_command(conn, "mark switch-all", NULL);

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
    g_sprintf(command, "workspace %s", names[i]);
    i3ipc_connection_command(conn, command, NULL);
    i3ipc_connection_command(conn, "move workspace to output left", NULL);
  }

  i3ipc_connection_command(conn, "[con_mark=\"switch_all\"] focus", NULL);
  i3ipc_connection_command(conn, "unmark switch-all", NULL);

  g_free(reply);
  g_object_unref(conn);
  g_free(root);
  g_object_unref(parser);
  g_object_unref(reader);
  g_free(names);

  return EXIT_SUCCESS;
}
