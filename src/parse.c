node_t *expr();
node_t *block();

node_t *base(int type) {
    token_t t = skip(type);
    node_t *r = node(type);
    r->sym = t.sym;
    return r;
}

node_t *id() {
    return base(Id);
}

node_t *str() {
    return base(Str);
}

node_t *num() {
    return base(Num);
}

node_t *exp_list(int type, char lpair, char rpair) {
    node_t *r = node(type);
    r->list = list();
    skip(lpair);
    while (tk.type != rpair) {
        list_add(r->list, expr());
    }
    skip(rpair);
    list_reverse(r->list);
    return r;
}

// args = (expr*)
node_t *args() {
    return exp_list(Args, '(', ')');
}

// array = [ expr* ]
node_t *array() {
    return exp_list(Array, '[', ']');
}

// term =  id ([expr] | . id | args )*
node_t *term() {
    node_t *r = node(Term);
    node_t *nid = id(), *t = nid;
    r->list = list();
    list_add(r->list, nid);
    while (strchr("[.(", tk.type)) {
        if (tk.type == '[') { // array member
            next();
            node_t *e=expr();
            skip(']');
            list_add(r->list, op1('[', e));
        } else if (tk.type == '.') { // object member
            next();
            node_t *mid = id();
            list_add(r->list, op1('.', mid));
        } else if (tk.type == '(') { // function call
            list_add(r->list, args('(', ')'));
        }
    }
    list_reverse(r->list);
    return r;
}

// params = (id(:expr)?)*
node_t *params() {
    node_t *r = node(Params);
    r->list = list();
    while (tk.type != ')') { 
        node_t *nid = id();
        node_t *e = NULL;
        if (tk.type == ':') {
            skip(':');
            e = expr();
        }
        list_add(r->list, op2(Param, nid, e));
    }
    list_reverse(r->list);
    return r;
}

// item = Num | Str | fn | array | block | ( expr ) | term
node_t *item() {
    if (tk.type == Num) {
        return num();
    } else if (tk.type == Str) {
        return str();
    } else if (match("fn")) { // fn (params) block
        next();
        skip('(');
        node_t *p1 = params();
        skip(')');
        node_t *b1 = block();
        return op2(Function, p1, b1);
    } else if (tk.type == '[') { // array
        return array();
    } else if (tk.type == '{') { // block
        return op1(Item, block()); // Item 中的 block 需包起來，方便識別
        // return block();
    } else if (tk.type == '(') { // ( expr )
        skip('(');
        node_t *e = expr();
        skip(')');
        return e;
    } else {
        return term();
    }
}

// expr = item (op2 expr)?
node_t *expr() {
    node_t *r = item();
    if (is_op2(tk.type)) {
        token_t op = next();
        node_t *e = expr();
        r = op2(op.type, r, e);
    }
    return r;
}

// stmt = while expr block          | 
//        if expr stmt (else stmt)? |
//        for id in expr stmt       | 
//        return exp                |
//        (id=)? expr
node_t *stmt() {
    node_t *e, *s, *r=node(Stmt);
    if (match("while")) { // while expr stmt
        next();
        e = expr();
        s = stmt();
        r->node = op2(While, e, s);
    } else if (match("if")) { // if expr stmt (else stmt)?
        next();
        e = expr();
        s = stmt();
        node_t *s2 = NULL;
        if (match("else")) {
            next();
            s2 = stmt();
        }
        r->node = op3(If, e, s, s2);
    } else if (match("for")) { // for id in expr stmt
        next();
        node_t *nid = id();
        if (match("in") || match("of")) {
            token_t optk = next();
            int op = (tk_match(optk, "in"))?ForIn:ForOf;
            e = expr();
            s = stmt();
            r->node = op3(op, nid, e, s);
        } else if (match("=")) {
            next();
            node_t *step=NULL, *from, *to;
            from = expr();
            skip_str("to");
            to = expr();
            if (match("step")) {
                next();
                step = expr();
            }
            s = stmt();
            r->node = op5(ForTo, nid, from, to, step, s);
        }
    } else if (match("return")) { // return exp
        next();
        e = expr();
        r->node = op1(Return, e);
    } else if (match("continue")) { // continue
        next();
        r->node = op0(Continue);
    } else if (match("break")) { // break
        next();
        r->node = op0(Break);
    } else {
        scan_save();
        bool is_exp = true;
        if (tk.type == Id) {
            token_t tid = skip(Id);
            if (tk.type=='=' || tk.type==':') { // (strchr("=:", tk.type)) {
                char op = tk.type;
                next();
                e = expr();
                // code gen id=exp
                node_t *nid = node(Id);
                nid->sym = tid.sym;
                r->node = op2(op, nid, e);
                is_exp = false;
            } else {
                scan_restore();
            }
        }
        if (is_exp) r->node = expr();
    }
    return r;
}

// stmts = stmt*
node_t *stmts() {
    node_t *r = node(Stmts);
    r->list = list();
    while (tk.type != End && tk.type != '}') {
        list_add(r->list, stmt());
    }
    list_reverse(r->list);
    return r;
}

// block = { stmts }
node_t *block() {
    node_t *r = node(Block);
    skip('{');
    r->node = stmts();
    skip('}');
    return r;
}

// program = stmts
node_t *parse(char *source) {
    p = source;
    next();
    return stmts();
}
