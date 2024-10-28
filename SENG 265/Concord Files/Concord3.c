
  concord3.c
 
  Fall 2022.
 

#include assert.h
#include stdio.h
#include ctype.h
#include stdlib.h
#include string.h
#include strings.h
#include emalloc.h
#include seng265-list.h

helper function that prints linked list
void print_word(node_t node, void arg)
{
    char format = (char)arg;
    printf(format, node-text);
}

frees list
void free_them(node_t head)
{
        node_t temp_node = head;
    while (temp_node != NULL) {
        assert(temp_node != NULL);
        head = remove_front(head);
        free(temp_node);
        temp_node = head;
    }

    assert(head == NULL);
}

checks version
void version_check()
{
  char version[] = ;
    fgets(version,3,  stdin);
    if(strstr(version,1)) {
        printf(Input is version 1, concord3 expected version 2n);
                exit(0);
    }
}

stores exlusion words
node_t store_excl()
{
          int num_excl = -3;
        char str[40];

        node_t head_excl= NULL;
        node_t temp_excl_node = NULL;

        for(; !strstr(fgets( str, 40, stdin), ) ; num_excl++) {
                temp_excl_node = new_node(str);
                head_excl = add_front(head_excl, temp_excl_node);
        }

        return head_excl;
}

verifes word is not excluded
void contains(node_t node, void wrd)
{
        if((char)wrd==NULL) {
                return;
        }

        char wrd_loc = strcasestr(node-text, (char)wrd);
    if(wrd_loc!=NULL && !isalpha(wrd_loc[-1]) && !isalpha(wrd_loc[strlen((char)wrd)])) {
                (char)wrd=NULL;
        }
}

adds index words alphabetically
node_t add_alphabetically(node_t head_node, node_t temp_node)
{
        if(head_node!=NULL && strncasecmp(temp_node-text, head_node-text,40)0) {
            printf(slots new word at front of index words alphabetically);
            temp_node-next=head_node;
            return temp_node;
        }

        for( node_t node = head_node; node!=NULL; node=node-next) {
                if(node-next==NULL) {
                printf(slots new word at end of index words alphabetically);
                        node-next = temp_node;
            return head_node;
        }

                else if(strncasecmp(temp_node-text, node-next-text,40)0) {
                        printf(slots new word in index words alphabetically);
                        temp_node-next=node-next;
                        node-next=temp_node;
                        return head_node;
                }
        }

        return temp_node;
}

capitalizes each word in string
void string_toupper(char wrd, int wrd_len) {
        for(int i =0; iwrd_len; i++) {
                wrd[i]=toupper(wrd[i]);
        }

}

strips and prints left side of string
void print_left(char snip_str)
{
    size_t left_len = strlen(snip_str);
        if(left_len=19) {
                printf(%28s, snip_str);
                return;
        }
        for(int i = 0; snip_str[0]!=' '  left_len20 ; left_len--, i++) {
           ++snip_str;
    }

    printf(%28s, snip_str);
}

avoids printing any extra spaces at end of line
int snip_space(char str, size_t str_len)
{
        for(int i=str_len; ; i--) {
                if(isalpha(str[i-1])) {
                        return i;
                }
        }
}

strips and prints right side of string
void print_right(char snip_str, int start_len)
{
    size_t right_len = strlen(snip_str);

        if(right_len!=0 && right_len=30 && !isalpha(snip_str[right_len-2])) {
        printf(%.s, snip_space(snip_str, right_len), snip_str);
                printf(n);
                return;
    }

        else if(right_len=31-start_len) {
        printf(%s, snip_str);
        return;
    }

        make stop at next word below threshold
        for(int i = right_len-1; snip_str[i]!=' '  right_len31-start_len; right_len--, i--) {
                snip_str[i]=' ';
    }

    printf(%.s, (int)right_len-1, snip_str);
        printf(n);
}


seperates index word from line
void print_line(char line, char wrd, size_t wrd_len)
{
        char left = ; char cap = ; char right = ;
        char space = ;

        if(wrd[-1]!=' ' && wrd[wrd_len]==' ') {
                wrd[wrd_len]='-';
                cap   = strtok(line,-);
                right = strtok(NULL, -);
                space= ;
        }

        else if(wrd[-1]==' ' && wrd[wrd_len]!=' ') {
                wrd[-1]='-';
                left   = strtok(line,-);
        cap = strtok(NULL, -);
        }

        else if(wrd[-1]!=' ' && wrd[wrd_len]!=' ' ) {
                cap = line;
        }

        else {
                wrd[-1]= '-'; wrd[wrd_len]='-';
                left = strtok(line,-);
                cap  = strtok(NULL,-);
                right = strtok(NULL, -);
                space =  ;
        }

        string_toupper(cap, wrd_len);

        print_left(left);
        printf( %s%s, cap, space);
        print_right(right, wrd_len);

}

runs through each index word and prints it for all its lines
void print_output(node_t idx_head, node_t line_head)
{
        for(node_t idx_node = idx_head; idx_node!=NULL; idx_node=idx_node-next) {
                for(node_t line_node = line_head; line_node!=NULL; line_node=line_node-next)  {
                        char temp_line[100];
            strncpy(temp_line,line_node-text,100);
                char idx_loc = strcasestr(temp_line,idx_node-text);
            size_t idx_len = strlen(idx_node-text);

                        for(char line_pos; idx_loc; idx_loc=strcasestr(line_pos, idx_node-text) ) {
                                if(!isalpha(idx_loc[-1]) && !isalpha(idx_loc[idx_len])) {
                        print_line(temp_line, idx_loc, idx_len);
                                        break;
                                }

                                line_pos=idx_loc+idx_len;
            }
                }
        }
}

int main(int argc, char argv[])
{

        version_check();
    node_t head_excl = store_excl();

        char str[100];

    node_t temp_input_node, temp_output_node = NULL;
        node_t head_input, head_output = NULL;

    for(; fgets( str, 100, stdin) ; ) {
        temp_input_node = new_node(str);
        head_input = add_front(head_input, temp_input_node);
        temp_output_node = new_node(str);
        head_output = add_end(head_output, temp_output_node);
    }

        node_t temp_idx_node = NULL;
        node_t head_idx = NULL;

        for(node_t node = head_input; node!=NULL; node=node-next) {
            char cur_word =strtok( node-text,  n);
                        for(; cur_word!=NULL; cur_word = strtok(NULL,  n)) {
                apply(head_excl, contains, &cur_word);
                apply(head_idx, contains, &cur_word);
                if(cur_word!=NULL) {
                        temp_idx_node = new_node(cur_word);
                                head_idx = add_alphabetically(head_idx, temp_idx_node);
                        }
                }
        }

        print_output(head_idx, head_output);

        free_them(head_excl);
        free_them(head_idx);
        free_them(head_input);
        free_them(head_output);

        exit(0);
}
