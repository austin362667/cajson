node_t *expr();
node_t *block();

// args = (expr {,expr}*)
node_t *args() {
    skip('(');
    while (tk.type != ')') {
        expr();
        if (tk.type != ')') skip(',');
    }
    skip(')');
}

node_t *array() {
    skip('[');
    while (tk.type != ']') {
        expr();
        if (tk.type != ']') skip(',');
    }
    skip(']');
}

// term =  Name ([expr] | . Name | args )*
node_t *term() {
    skip(Id);
    while (strchr("[.(", tk.type)) {
        if (tk.type == '[') { // array member
            next();
            expr();
            skip(']');
        } else if (tk.type == '.') { // object member
            next();
            skip(Id);
        } else if (tk.type == '(') { // function call
            args('(', ')');
        }
    }
}

// item = Num | Str | fn | array | block | ( expr ) | term
node_t *item() {
    if (tk.type == Num || tk.type == Str) { // Num | Str  // n = node(tk.type); n->sym = tk.sym;
        next();
    } else if (match("fn")) { // fn (id {,id}*) block
        next();
        skip('(');
        while (tk.type != ')') { 
            skip(Id);
            if (tk.type != ')') skip(',');
        }
        skip(')');
        block();
    } else if (tk.type == '[') { // array
        array();
    } else if (tk.type == '{') { // block
        block();
    } else if (tk.type == '(') { // ( expr )
        skip('(');
        expr();
        skip(')');
    } else {
        term();
    }
}
// expr = item (op2 expr)*
node_t *expr() {
    item();
    while (is_op2(tk.type)) {
        next();
        expr();
    }
}

// stmt = WHILE | IF | RETURN | (id=)? EXP
node_t *stmt() {
    if (match("while")) { // while expr stmt
        next();
        expr();
        block();
    } else if (match("if")) { // if expr stmt (else stmt)?
        next();
        expr();
    } else if (match("return")) { // return exp
        next();
        expr();
    } else {
        scan_save();
        if (tk.type == Id) {
            skip(Id);
            if (tk.type=='=' || tk.type==':') { // (strchr("=:", tk.type)) {
                next();
                expr();
                return NULL;
            } else {
                scan_restore();
            }
        }
        expr();
    }
}

// stmts = stmt*
node_t *stmts() {
    while (tk.type != End && tk.type != '}') {
        stmt();
    }
}

// block = { stmts }
node_t *block() {
    skip('{');
    stmts();
    skip('}');
}

// program = stmts
node_t *parse(char *source) {
    p = source;
    next();
    return stmts();
}
