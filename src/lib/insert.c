/** MIT License
  *
  * Copyright (c) 2018 Abdullah Emad
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy
  * of this software and associated documentation files (the "Software"), to deal
  * in the Software without restriction, including without limitation the rights
  * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  * copies of the Software, and to permit persons to whom the Software is
  * furnished to do so, subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all
  * copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  * SOFTWARE.
  *
  */

/**
  * Code for handling the insertions in the B-Tree
  */
#include "lib.h"

/**
  * cbt_node_t*, cbt_entry_t* -> cbt_node_t*
  * EFFECT: Given the root of the tree, inserts the given entery in the appropriate location in the leaf
  * MODIFIES: Tree
  * RETURNS: void
  *
  * cranbtree_t* bt: btree struct pointer
  * cbt_node_t* root: the root of the tree
  * cbt_entry_t* entry: entry to be inserted in the tree
  * int n: the size of the entries array
  * int level: the current tree level. initially zero
  */
static void bt_insert_helper(cranbtree_t * bt, cbt_node_t * root,
			     cbt_entry_t * entry)
{

	assert(bt != NULL);
	/*the first insertion in the tree */
	if (root == NULL)
	{
		root = bt_create_node(bt->n);
		assert(root != NULL);
		node_insert_entry(root, entry, true, bt->n);
		bt->root = root;
		bt->max_key = entry->key;
		bt->min_key = entry->key;
		return;
	}
	/* we are at the root level */
	if (is_root(bt, root))
	{
		root = split_full_root(root, bt->n);
		/*splits the root if it needs splitting */
		if (bt->root != root)
		{
			/*restarts execution with the new root */
			bt->root = root;
			bt_insert_helper(bt, bt->root, entry);
			return;
		}
	}

	int child_index = get_next_node_index(root, entry->key, bt->n);

	/*this is a leaf (can be a root and a leaf at the same time) */
	if (is_leaf(root->children[child_index]))
	{
		node_insert_entry(root, entry, true, bt->n);
	}
	else
	{
		cbt_node_t *splitted_node = NULL;
		cbt_entry_t *median_entry =
		    bt_split_child(root->children[child_index], &splitted_node,
				   bt->n);

		/* a split happened */
		if (median_entry != NULL)
		{
			int splitted_node_ptr_index =
			    node_insert_entry(root, median_entry, true, bt->n);
			root->children[splitted_node_ptr_index] = splitted_node;
			child_index = get_next_node_index(root, entry->key, bt->n);	/*recalculates the path */
		}

		bt_insert_helper(bt, root->children[child_index], entry);
	}
}

/*
 * cbt_node_t*, int -> cbt_node_t*
 *
 * EFFECTS: Creates a new root for the tree if the node needs to be splitted
 * MODIFIES: cbt_node_t** root
 * RETURNS: the new node that was created in case of a split, otherwise returns the old node
 *
 */
static cbt_node_t *split_full_root(cbt_node_t * old_root, int n)
{
	cbt_node_t *splitted_node = NULL;
	cbt_entry_t *root_entry = bt_split_child(old_root, &splitted_node, n);

	/*creates new root and restarts execution */
	if (root_entry != NULL)
	{
		cbt_node_t *new_root = bt_create_node(n);

		assert(new_root != NULL);
		node_insert_entry(new_root, root_entry, true, n);
		new_root->children[0] = old_root;
		new_root->children[1] = splitted_node;
		return new_root;
	}
	else
	{
		return old_root;
	}

}

 /*
  * cbt_node_t*, cbt_node_t**, int -> cbt_entry_t*
  * splits a given node into two and returns the entry that should be inserted in the parent node
  * return NULL if no split is needed. i.e: the node is not full
  * REQUIRES: node to be sorted
  */
static cbt_entry_t *bt_split_child(cbt_node_t * node,
				   cbt_node_t ** splitted_node, int n)
{
	/*the node does not need to be splitted */
	if (!is_full_node(node, n))
	{
		return NULL;
	}

	/*splits the node */
	cbt_node_t *new_node = bt_create_node(n);
	int borrow_n = ceil_fn(((double)n) / 2.0);
	int new_node_itr = 0;

	/*edge case for the pointers */
	new_node->children[0] = node->children[borrow_n];
	node->children[borrow_n] = NULL;
	for (int i = borrow_n; i < n; i++)
	{
		/*copies over the entries and pointers from the old node */
		new_node->entry[new_node_itr++] = node->entry[i];
		new_node->children[new_node_itr] = node->children[i + 1];
		node->entry[i] = NULL;
		node->children[i + 1] = NULL;
	}
	/*updates the length */
	new_node->len = n / 2;
	node->len = n / 2;

	cbt_entry_t *entry = node->entry[borrow_n - 1];

	node->entry[borrow_n - 1] = NULL;
	*(splitted_node) = new_node;
	return entry;

}
