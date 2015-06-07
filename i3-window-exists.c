#include <stdlib.h>
#include <glib/gprintf.h>
#include <json-glib/json-glib.h>
#include <getopt.h>

struct Node
{
  const gchar* type;
  const gchar* window_class;
  const gchar* window_instance;
  unsigned int node_count;
  struct Node* nodes;
};

void read_tree(JsonReader* reader, struct Node* node)
{
  int i, j, z;

  node->type = NULL;
  node->window_class = NULL;
  node->window_instance = NULL;
  node->node_count = 0;
  node->nodes = NULL;

  /*
   * In >4.8 type is a string
   */
  json_reader_read_member(reader, "type");
  node->type = json_reader_get_string_value(reader);
  json_reader_end_member(reader);

  if (json_reader_read_member(reader, "window_properties"))
  {
    json_reader_read_member(reader, "class");
    node->window_class = json_reader_get_string_value(reader);
    json_reader_end_member(reader);

    json_reader_read_member(reader, "instance");
    node->window_instance = json_reader_get_string_value(reader);
    json_reader_end_member(reader);
  }
  json_reader_end_member(reader);

  json_reader_read_member(reader, "nodes");
  if (json_reader_is_array(reader))
  {
    j = json_reader_count_elements(reader);
    node->node_count += j;
    node->nodes = (struct Node*) g_malloc(sizeof(struct Node) * j);

    for (i = 0; i < j; ++i)
    {
      json_reader_read_element(reader, i);
      read_tree(reader, &node->nodes[i]);
      json_reader_end_element(reader);
    }
  }
  json_reader_end_member(reader);

  json_reader_read_member(reader, "floating_nodes");
  if (json_reader_is_array(reader))
  {
    z = json_reader_count_elements(reader);
    node->node_count += z;
    node->nodes = (struct Node*) g_realloc(node->nodes, sizeof(struct Node) * (j + z));

    for (i = 0; i < z; ++i)
    {
      json_reader_read_element(reader, i);
      read_tree(reader, &node->nodes[j + i]);
      json_reader_end_element(reader);
    }
  }
  json_reader_end_member(reader);
}

void free_tree(struct Node* node) {
  unsigned int i;

  for (i = 0; i < node->node_count; ++i)
  {
    free_tree(&node->nodes[i]);
  }

  if (node->nodes)
  {
    g_free(node->nodes);
  }
}

int find_node(struct Node node, const gchar* window_class, const gchar* window_instance)
{
  unsigned int i, x, y;
  g_printf("Node type: %s\n", node.type);
  g_printf("Children: %d\n", node.node_count);

  if (node.window_class)
  {
    g_printf("Class: %s\n", node.window_class);
  }

  if (node.window_instance)
  {
    g_printf("Instance: %s\n", node.window_instance);
  }

  x = node.window_class && window_class && g_strcmp0(window_class, node.window_class) == 0;
  y = node.window_instance && window_instance && g_strcmp0(window_instance, node.window_instance) == 0;

  if ((window_instance && window_class && x && y)
      || (window_class && x)
      || (window_instance && y))
  {
    return 1;
  }

  for (i = 0; i < node.node_count; ++i)
  {
    if (find_node(node.nodes[i], window_class, window_instance))
    {
      return 1;
    }
  }

  return 0;
}


int main (int argc, char** argv)
{
  static int help = 0;
  const gchar* window_class = NULL;
  const gchar* window_instance = NULL;
  int ret;
  gchar *reply;
  JsonParser* parser;
  JsonReader* reader;
  JsonNode* root;
  struct Node root_node;

  static struct option long_options[] = {
    {"window_class", required_argument, NULL, 'c'},
    {"window_instance", required_argument, NULL, 'i'},
    {"help", no_argument, &help, 1}
  };

  while (1)
  {
    int i = 0;
    int c = getopt_long(argc, argv, "c:i:", long_options, &i);

    if (c == -1) break;

    switch (c) {
      case 'c':
        window_class = optarg;
        break;
      case 'i':
        window_instance = optarg;
        break;
    }
  }

  if (help > 0)
  {
    printf("\
Usage: %s [OPTION] COMMAND\n\
\n\
  -c, --window_class     \n\
  -i, --window_instance  \n\
      --help             Display this help and exit\n\
      ", argv[0]);

    exit(0);
  }

  if (window_instance == NULL && window_class == NULL)
  {
    printf("Instance or class required.");
    exit(1);
  }

  g_spawn_command_line_sync("i3-msg -t get_tree", &reply, NULL, NULL, NULL);

  parser = json_parser_new();
  if (!json_parser_load_from_data(parser, reply, -1, NULL))
  {
    exit(1);
  }

  root = json_parser_get_root(parser);
  reader = json_reader_new(root);

  read_tree(reader, &root_node);

  if (find_node(root_node, window_class, window_instance))
  {
    ret = EXIT_SUCCESS;
  }
  else
  {
    ret = EXIT_FAILURE;
  }

  free_tree(&root_node);
  g_object_unref(reader);
  g_object_unref(parser);
  g_free(reply);

  return ret;
}
