// jsparse.js, based on jslint.js
// 2007-12-30
/*

Based on JSLint by Douglas Crockford.
Copyright (c) 2002 Douglas Crockford  (www.JSLint.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

    var obj = parse(source, filename [, option]);

### Arguments ###

* _source_: A string containing JavaScript source code.
* _filename_: A string used in error messages
* _option_: Don't use this

### Return value ###

An object containing the following properties:

* _statements_: An array representing the parse tree. Can be passed
  to other functions, like [[$curdir.unparse]].
* _functions_: An object containing function declarations that
  are not anonymous.
* _p1implied_: An object containing global variables that are
  used but not defined in the source code.
* _p2implied_: An object containing global variables that are
  used but not defined in the source code.

If the program uses a global variable named _b.c_, then
_p1implied_ will contain a property named _b_, containing
a property named _c_, which is an empty object.

If the program, inside a _with_ block using a global variable
_a_, references an unqualified variable named _b_, then
_p1implied_ will contain a property named _a_, containing
a property named _(with)_, containing a property named _b_,
being an empty object.

*/

// We build the application inside a function so that we produce only a single
// global variable. The function will be invoked, its return value is the JSLINT
// function itself.

(function () {

// These are all of the JSLint options.

      var allOptions = {
            bitwise    : true, // if bitwise operators should not be allowed
            cap        : true, // if upper case HTML should be allowed
            debug      : true, // if debugger statements should be allowed
            eqeqeq     : true, // if === should be required
            evil       : true, // if eval should be allowed
            forin      : true, // if for in statements must filter
            fragment   : true, // if HTML fragments should be allowed
            laxbreak   : true, // if line breaks should not be checked
            nomen      : true, // if names should be checked
            on         : true, // if HTML event handlers should be allowed
            passfail   : true, // if the scan should stop on first error
            plusplus   : true, // if increment/decrement should not be allowed
            undef      : true, // if variables should be declared before used
            white      : true  // if strict whitespace rules apply
        },

	functions,
        filename,
        funct,          // The current function
        global,         // The global object
        globals,        // The current globals
        implied,        // Implied globals used in current context
        p2implied,      // Implied globals used inside functions
        funcnest,       // Current function nesting level
        inblock,
        lines,
        lookahead,
        nexttoken,
        noreach,
        option,
        prereg,
        prevtoken,
        scope,      // The current scope
        src,
        stack,

// standard contains the global names that are provided by the
// ECMAScript standard.

        standard = {
            Array               : true,
            Boolean             : true,
            Date                : true,
            decodeURI           : true,
            decodeURIComponent  : true,
            encodeURI           : true,
            encodeURIComponent  : true,
            Error               : true,
            escape              : true,
            'eval'              : true,
            EvalError           : true,
            Function            : true,
            isFinite            : true,
            isNaN               : true,
            Math                : true,
            Number              : true,
            Object              : true,
            parseInt            : true,
            parseFloat          : true,
            RangeError          : true,
            ReferenceError      : true,
            RegExp              : true,
            String              : true,
            SyntaxError         : true,
            TypeError           : true,
            unescape            : true,
            URIError            : true
        },

        syntax = {},
        token,

// token
        tx = /^\s*([(){}\[.,:;\'\"~]|\](\]>)?|\?>?|==?=?|\/(\*|=|\/)?|\*[\/=]?|\+[+=]?|-[\-=]?|%[=>]?|&[&=]?|\|[|=]?|>>?>?=?|<([\/=%\?]|\!(\[|--)?|<=?)?|\^=?|\!=?=?|[a-zA-Z_$][a-zA-Z0-9_$]*|[0-9]+([xX][0-9a-fA-F]+|\.[0-9]*)?([eE][+\-]?[0-9]+)?)/,
// star slash
        lx = /\*\/|\/\*/,
// identifier
        ix = /^([a-zA-Z_$][a-zA-Z0-9_$]*)$/;

    var hasOwnProperty=Object.prototype.hasOwnProperty;

    function object(o) {
        function F() {}
        F.prototype = o;
        return new F();
    }

    function combine(o) {
        var n;
        for (n in o) if (hasOwnProperty.call(o,n)) {
            this[n] = o[n];
        }
    };

    function isAlpha(s) {
        return (s >= 'a' && s <= 'z\uffff') ||
            (s >= 'A' && s <= 'Z\uffff');
    };


    function isDigit(s) {
        return (s >= '0' && s <= '9');
    };


    function supplant(o) {
        return this.replace(/\{([^{}]*)\}/g, function (a, b) {
            var r = o[b];
            return typeof r === 'string' || typeof r === 'number' ? r : a;
        });
    };

// Produce an error warning.

    function quit(m, l, ch) {
        throw new Error(m);
    }

    function warning(m, t, a, b, c, d) {
        print(filename+":"+(t || token).line+": "+m.replace('{a}',a).replace('{b}',b).replace('{c}',c).replace('{d}',d));
        print("\n");

        var ch, l, w;
        t = t || nexttoken;
        if (t.id === '(end)') {
            t = token;
        }
        l = t.line || 0;
        ch = t.from || 0;
        w = {
            id: '(error)',
            raw: m,
            evidence: lines[l] || '',
            line: l,
            character: ch,
            a: a,
            b: b,
            c: c,
            d: d
        };
        w.reason = supplant.call(m, w);
        JSLINT.errors.push(w);
        if (option.passfail) {
            quit('Stopping. ', l, ch);
        }
        return w;
    }

    function warningAt(m, l, ch, a, b, c, d) {
        return warning(m, {
            line: l,
            from: ch
        }, a, b, c, d);
    }

    function error(m, t, a, b, c, d) {
	throw(filename+":"+(t || token).line+": "+m.replace('{a}',a).replace('{b}',b).replace('{c}',c).replace('{d}',d));
//        print("Error: ");
        var w = warning(m, t, a, b, c, d);
        quit("Stopping, unable to continue.", w.line, w.character);
    }

    function errorAt(m, l, ch, a, b, c, d) {
        return error(m, {
            line: l,
            from: ch
        }, a, b, c, d);
    }

    function expected(what) {
      if (nexttoken.id === '(end)') {
        error("Unexpected early end of program.", token);
      } else {
        error("Expected {a} and instead saw '{b}'.",
               nexttoken, what, nexttoken.value);
      }
    }

// lexical analysis

    var lex = function () {
        var character, from, line, s;

// Private lex methods

        function nextLine() {
            line += 1;
            if (line >= lines.length) {
                return false;
            }
            character = 0;
            s = lines[line];
            return true;
        }

// Produce a token object.  The token inherits from a syntax symbol.

        function it(type, value) {
            var i, t;
            if (type === '(punctuator)' ||
                    (type === '(identifier)' && hasOwnProperty.call(syntax, value))) {
                t = syntax[value];

// Mozilla bug workaround.

                if (!t.id) {
                    t = syntax[type];
                }
            } else {
                t = syntax[type];
            }
            t = object(t);
            t.value = value;
            t.line = line;
            t.character = character;
            t.from = from;
            i = t.id;
            if (i !== '(endline)') {
                prereg = i &&
                        (('(,=:[!&|?{};'.indexOf(i.charAt(i.length - 1)) >= 0) ||
                        i === 'return');
            }
            return t;
        }

// Public lex methods

        return {
            init: function (source) {
                if (typeof source === 'string') {
                    lines = source.
                        replace(/\r\n/g, '\n').
                        replace(/\r/g, '\n').
                        split('\n');
                } else {
                    lines = source;
                }
                line = -1;
                nextLine();
                from = 0;
            },

// token -- this is called by advance to get the next token.

            token: function () {
                var b, c, captures, d, depth, high, i, l, low, q, t;

                function match(x) {
                    var r = x.exec(s), r1;
                    if (r) {
                        l = r[0].length;
                        r1 = r[1];
                        c = r1.charAt(0);
                        s = s.substr(l);
                        character += l;
                        from = character - r1.length;
                        return r1;
                    }
                }

                function string(x) {
                    var c, j, r = '';

                    function esc(n) {
                        var i = parseInt(s.substr(j + 1, n), 16);
                        j += n;
                        character += n;
                        c = String.fromCharCode(i);
                    }
                    j = 0;
                    for (;;) {
                        if (j >= s.length) {
                            j = 0;
                            errorAt("Unclosed string.", line, from);
                        }
                        c = s.charAt(j);
                        if (c === x) {
                            character += 1;
                            s = s.substr(j + 1);
                            return it('(string)', r, x);
                        }
                        if (c < ' ') {
                            if (c === '\n' || c === '\r') {
                                break;
                            }
                        } else if (c === '\\' && j === s.length-1) {
                            if (!nextLine()) {
                                error("Unexpected early end of program.", line, character);
                            }
                            j=0;
                            continue;
                        } else if (c === '\\') {
                            j += 1;
                            character += 1;

                            c = s.charAt(j);
                            switch (c) {
                            case '\\':
                            case '\'':
                            case '"':
                            case '/':
                                break;
                            case 'b':
                                c = '\b';
                                break;
                            case 'f':
                                c = '\f';
                                break;
                            case 'n':
                                c = '\n';
                                break;
                            case 'r':
                                c = '\r';
                                break;
                            case 't':
                                c = '\t';
                                break;
                            case 'u':
                                esc(4);
                                break;
                            case 'v':
                                c = '\v';
                                break;
                            case 'x':
                                esc(2);
                                break;
                            }
                        }
                        r += c;
                        character += 1;
                        j += 1;
                    }
                }

                for (;;) {
                    if (!s) {
                        return it(nextLine() ? '(endline)' : '(end)', '');
                    }
                    t = match(tx);
                    if (!t) {
                        t = '';
                        c = '';
                        while (s && s < '!') {
                            s = s.substr(1);
                        }
                        if (s) {
                            errorAt("Unexpected '{a}'.",
                                    line, character, s.substr(0, 1));
                        }
                    }

//      identifier

                    if (isAlpha(c) || c === '_' || c === '$') {
                        return it('(identifier)', t);
                    }

//      number

                    if (isDigit(c)) {
                        return it('(number)', t);
                    }

//      string

                    switch (t) {
                    case '"':
                    case "'":
                        return string(t);

//      // comment

                    case '//':
                        s = '';
                        break;

//      /* comment

                          case '/*':
                              for (;;) {
                                  i = s.search(lx);
                                  if (i >= 0) {
                                      break;
                                  }
                                  if (!nextLine()) {
                                      errorAt("Unclosed comment.", line, character);
                                  }
                              }
                              character += i + 2;
                              if (s.substr(i, 1) === '/') {
                                  errorAt("Nested comment.", line, character);
                              }
                              s = s.substr(i + 2);
                              break;

//      */

                    case '*/':
                        return {
                            value: t,
                            type: 'special',
                            line: line,
                            character: character,
                            from: from
                        };

                    case '':
                        break;
//      /
                    case '/':
                        if (prereg) {
                            depth = 0;
                            captures = 0;
                            l = 0;
                            for (;;) {
                                b = true;
                                c = s.charAt(l);
                                l += 1;
                                switch (c) {
                                case '':
                                    errorAt("Unclosed regular expression.", line, from);
                                    return;
                                case '/':
                                    c = s.substr(0, l - 1);
				    var flags="";
                                    for (;;) {
                                      if (s.charAt(l) === 'g') {
					flags+="g";
                                        l += 1;
                                      } else if (s.charAt(l) === 'i') {
					flags+="i";
                                        l += 1;
                                      } else if (s.charAt(l) === 'm') {
					flags+="m";
                                        l += 1;
                                      } else {
                                        break;
                                      }
                                    }
                                    character += l;
                                    s = s.substr(l);
				    var ret=it('(regex)', c);
                                    ret.flags=flags;
				    return ret;
                                case '\\':
                                    l += 1;
                                    break;
                                case '(':
                                    depth += 1;
                                    b = false;
                                    if (s.charAt(l) === '?') {
                                        l += 1;
                                        switch (s.charAt(l)) {
                                        case ':':
                                        case '=':
                                        case '!':
                                            l += 1;
                                            break;
                                        }
                                    } else {
                                        captures += 1;
                                    }
                                    break;
                                case ')':
                                    if (depth !== 0) {
                                        depth -= 1;
                                    }
                                    break;
                                case ' ':
                                    q = 1;
                                    while (s.charAt(l) === ' ') {
                                        l += 1;
                                        q += 1;
                                    }
                                    break;
                                case '[':
                                    if (s.charAt(l) === '^') {
                                        l += 1;
                                    }
                                    q = false;
klass:                              for (;;) {
                                        c = s.charAt(l);
                                        l += 1;
                                        switch (c) {
                                        case '[':
                                        case '^':
                                            q = true;
                                            break;
                                        case '-':
                                            if (q) {
                                                q = false;
                                            } else {
                                                q = true;
                                            }
                                            break;
                                        case ']':
                                            break klass;
                                        case '\\':
                                            l += 1;
                                            q = true;
                                            break;
                                        default:
                                            if (c < ' ') {
                                                errorAt(c ? "Control character in a regular expression" :
                                                    "Unclosed regular expression.", line, from + l);
                                            }
                                            q = true;
                                        }
                                    }
                                    break;
                                case ']':
                                case '?':
                                case '{':
                                case '}':
                                case '+':
                                case '*':
                                    break;
                                }
                                if (b) {
                                    switch (s.charAt(l)) {
                                    case '?':
                                    case '+':
                                    case '*':
                                        l += 1;
                                        if (s.charAt(l) === '?') {
                                            l += 1;
                                        }
                                        break;
                                    case '{':
                                        l += 1;
                                        c = s.charAt(l);
                                        l += 1;
                                        low = +c;
                                        for (;;) {
                                            c = s.charAt(l);
                                            if (c < '0' || c > '9') {
                                                break;
                                            }
                                            l += 1;
                                            low = +c + (low * 10);
                                        }
                                        high = low;
                                        if (c === ',') {
                                            l += 1;
                                            high = Infinity;
                                            c = s.charAt(l);
                                            if (c >= '0' && c <= '9') {
                                                l += 1;
                                                high = +c;
                                                for (;;) {
                                                    c = s.charAt(l);
                                                    if (c < '0' || c > '9') {
                                                        break;
                                                    }
                                                    l += 1;
                                                    high = +c + (high * 10);
                                                }
                                            }
                                        }
                                        if (s.charAt(l) === '}') {
                                            l += 1;
                                        }
                                        if (s.charAt(l) === '?') {
                                            l += 1;
                                        }
                                    }
                                }
                            }
                            c = s.substr(0, l - 1);
                            character += l;
                            s = s.substr(l);
                            return it('(regex)', c);
                        }
                        return it('(punctuator)', t);

//      punctuator

                    default:
                        return it('(punctuator)', t);
                    }
                }
            },

// skip -- skip past the next occurrence of a particular string.
// If the argument is empty, skip to just before the next '<' character.
// This is used to ignore HTML content. Return false if it isn't found.

            skip: function (p) {
                var i, t = p;
                if (nexttoken.id) {
                    if (!t) {
                        t = '';
                        if (nexttoken.id.substr(0, 1) === '<') {
                            lookahead.push(nexttoken);
                            return true;
                        }
                    } else if (nexttoken.id.indexOf(t) >= 0) {
                        return true;
                    }
                }
                token = nexttoken;
                nexttoken = syntax['(end)'];
                for (;;) {
                    i = s.indexOf(t || '<');
                    if (i >= 0) {
                        character += i + t.length;
                        s = s.substr(i + t.length);
                        return true;
                    }
                    if (!nextLine()) {
                        break;
                    }
                }
                return false;
            }
        };
    }();


    function addlabel(t, type) {
/*
              if (t === 'hasOwnProperty') {
                  error("'hasOwnProperty' is a really bad name.");
              }
*/
 
// Define t in the current function in the current scope.

        scope[t] = funct;
        funct[t] = type;
        if (hasOwnProperty.call(implied, t)) {
            delete implied[t];
        }
    }

// We need a peek function. If it has an argument, it peeks that much farther
// ahead. It is used to distinguish
//     for ( var i in ...
// from
//     for ( var i = ...

    function peek(p) {
        var i = p || 0, j = 0, t;

        while (j <= i) {
            t = lookahead[j];
            if (!t) {
                t = lookahead[j] = lex.token();
            }
            j += 1;
        }
        return t;
    }


    var badbreak = {
        ')': true,
        ']': true,
        '++': true,
        '--': true
    };

// Produce the next token. It looks for programming errors.

    function advance(id, t) {
        var l;

        prevtoken = token;
        token = nexttoken;
        for (;;) {
            nexttoken = lookahead.shift() || lex.token();
            if (nexttoken.id !== '(endline)') {
                if (id && token.id !== id)
                    expected(id);
                break;
            }
        }
    }


// This is the heart of JSLINT, the Pratt parser. In addition to parsing, it
// is looking for ad hoc lint patterns. We add to Pratt's model .fud, which is
// like nud except that it is only used on the first token of a statement.
// Having .fud makes it much easier to define JavaScript. I retained Pratt's
// nomenclature.

// .nud     Null denotation
// .fud     First null denotation
// .led     Left denotation
//  lbp     Left binding power
//  rbp     Right binding power

// They are key to the parsing method called Top Down Operator Precedence.

    function parse(rbp, initial) {
        var left;
        var o;
        if (nexttoken.id === '(end)') {
            error("Unexpected early end of program.", token);
        }
        advance();
        if (initial) {
            funct['(verb)'] = token.value;
        }
        if (initial === true && token.fud) {
            left = token.fud();
        } else {
            if (token.nud) {
                o = token.exps;
                left = token.nud();
            } else {
                if (nexttoken.type === '(number)' && token.id === '.') {
                    advance();
                    return token;
                } else {
                    expected("an identifier");
                }
            }
            while (rbp < nexttoken.lbp) {
                o = nexttoken.exps;
                advance();
                if (token.led) {
                    left = token.led(left);
                } else {
                    expected("an operator");
                }
            }
        }

	return left;
    }


// Parasitic constructors for making the symbols that will be inherited by
// tokens.

    function symbol(s, p) {
        var x = syntax[s];
        if (!x || typeof x !== 'object') {
            syntax[s] = x = {
                id: s,
                lbp: p,
                value: s
            };
        }
        return x;
    }


    function delim(s) {
        return symbol(s, 0);
    }


    function stmt(s, f) {
        var x = delim(s);
        x.identifier = x.reserved = true;
        x.fud = f;
        return x;
    }


    function blockstmt(s, f) {
        var x = stmt(s, f);
        x.block = true;
        return x;
    }


    function reserveName(x) {
        var c = x.id.charAt(0);
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            x.identifier = x.reserved = true;
        }
        return x;
    }


    function prefix(s, f) {
        var x = symbol(s, 150);
        reserveName(x);
        x.nud = (typeof f === 'function') ? f : function () {
	    this.right=parse(150);
	    return this;
        };
        return x;
    }


    function type(s, f) {
        var x = delim(s);
        x.type = s;
        x.nud = f;
        return x;
    }


    function reserve(s, f) {
        var x = type(s, f);
        x.identifier = x.reserved = true;
        return x;
    }


    function reservevar(s) {
        return reserve(s, function () {
            return this;
        });
    }


    function infix(s, f, p) {
        var x = symbol(s, p);
        reserveName(x);
        x.led = (typeof f === 'function') ? f : function (left) {
	    this.left=left;
	    this.right=parse(p);
	    return this;
        };
        return x;
    }


    function relation(s, f) {
        var x = symbol(s, 100);
        x.led = function (left) {
            var right = parse(100);
            if ((left && left.id=="NaN") || (right && right.id=="NaN")) {
            } else if (f) {
                f.apply(this, [left, right]);
            }
	    this.left=left;
	    this.right=right;
            return this;
        };
        return x;
    }


    function isPoorRelation(node) {
        return (node.type === '(number)' && !+node.value) ||
               (node.type === '(string)' && !node.value) ||
                node.type === 'true' ||
                node.type === 'false' ||
                node.type === 'undefined' ||
                node.type === 'null';
    }


    function assignop(s, f) {
        symbol(s, 20).exps = true;
        return infix(s, function (left) {
            if (s=='=' && left.firstglobal) {
              delete implied[left.value];
            }
            var l;
            if (left) {
                if (left.id === '.' || left.id === '[' ||
                        (left.identifier && !left.reserved)) {
                    
		  this.left=left;
		  this.right=parse(19);
		  return this;
                }
            }
            error("Bad assignment.", this);
        }, 20);
    }

    function bitwise(s, f, p) {
        var x = symbol(s, p);
        reserveName(x);
        x.led = (typeof f === 'function') ? f : function (left) {
	    this.left=left;
	    this.right=parse(p);
	    return this;
        };
        return x;
    }

    function bitwiseassignop(s) {
        symbol(s, 20).exps = true;
        return infix(s, function (left) {
            if (left) {
                if (left.id === '.' || left.id === '[' ||
                        (left.identifier && !left.reserved)) {
		  this.left=left;
		  this.right=parse(19);
		  return this;
                }
            }
            error("Bad assignment.", this);
        }, 20);
    }


    function suffix(s, f) {
        var x = symbol(s, 150);
        x.led = function (left) {
	    this.left=left;
	    return this;
        };
        return x;
    }


    function optionalidentifier() {
        if (nexttoken.identifier) {
            advance();
            return token.value;
        }
    }


    function identifier() {
        var i = optionalidentifier();
        if (i) {
            return i;
        }
        if (token.id === 'function' && nexttoken.id === '(') {
        } else {
            expected("an identifier");
        }
    }

    function reachable(s) {
        var i = 0, t;
        if (nexttoken.id !== ';' || noreach) {
            return;
        }
        for (;;) {
            t = peek(i);
            if (t.reach) {
                return;
            }
            if (t.id !== '(endline)') {
                break;
            }
            i += 1;
        }
    }


    function statement() {
        var r, s = scope, t = nexttoken;

// We don't like the empty statement.
        if (t.id === ';') {
            advance(';');
            return;
        }

// Is this a labelled statement?

        if (t.identifier && !t.reserved && peek().id === ':') {
            advance();
            advance(':');
            scope = object(s);
            addlabel(t.value, 'label');
            nexttoken.label = t.value;
            t = nexttoken;
        }

// Parse the statement.

        r = parse(0, true);

// Look for the final semicolon.

        if (!t.block) {
            if (nexttoken.id === ';') {
                advance(';');
            }
        }

// Restore the indentation.

        scope = s;
        return r;
    }


    function statements() {
        var a = [];
        while (!nexttoken.reach && nexttoken.id !== '(end)') {
            if (nexttoken.id === ';') {
                advance(';');
            } else {
                a.push(statement());
            }
        }
        return a;
    }


    function block(f) {
        var a, b = inblock, s = scope;
        inblock = f;
        if (f) {
            scope = object(scope);
        }
        var t = nexttoken;
        if (nexttoken.id === '{') {
            advance('{');
            if (nexttoken.id !== '}' || token.line !== nexttoken.line) {
                a = statements();
            }
            advance('}', t);
        } else {
            noreach = true;
            a = [statement()];
            noreach = false;
        }
        funct['(verb)'] = null;
        scope = s;
        inblock = b;
        return a;
    }

    function note_implied(token) {
      if (!hasOwnProperty.call(implied, token.value)) {
	implied[token.value]={};
        token.firstglobal=true;
      }
      token.global=implied[token.value];
    }
 
    function note_implied_property(top, left, e) {
      if (!hasOwnProperty.call(left.global, e)) {
	left.global[e]={};
      }
      top.global=left.global[e];
    }

// Build the syntax table by declaring the syntactic elements of the language.

    type('(number)', function(){return this});
    type('(string)', function(){return this});

    syntax['(identifier)'] = {
        type: '(identifier)',
        lbp: 0,
        identifier: true,
        nud: function () {
            var v = this.value,
                s = scope[v];

// The name is in scope and defined in the current function.

            if (s && (s === funct || s === funct['(global)'])) {

//      If we are not also in the global scope, change 'unused' to 'var',
//      and reject labels.

                if (!funct['(global)']) {
                    switch (funct[v]) {
                    case 'unused':
		      funct[v] = 'var';
                        break;
                    }
                }

// The name is not defined in the function.  If we are in the global scope,
// then we have an undefined variable.

            } else if (funct['(global)']) {
                note_implied(token);

// If the name is already defined in the current
// function, but not as outer, then there is a scope error.

            } else {
                switch (funct[v]) {
                case 'closure':
                case 'function':
                case 'var':
                case 'unused':
                case 'label':
                case 'outer':
                case true:
                    break;
                default:

// If the name is defined in an outer function, make an outer entry, and if
// it was unused, make it var.

                    if (s === true) {
                        funct[v] = true;
                    } else if (typeof s !== 'object') {
                        note_implied(token);
                    } else {
                        switch (s[v]) {
                        case 'function':
                        case 'var':
                        case 'unused':
                            s[v] = 'closure';
                            funct[v] = 'outer';
                            break;
                        case 'closure':
                        case 'parameter':
                            funct[v] = 'outer';
                            break;
                        }
                    }
                }
            }
            return this;//{value: this.value};
        },
        led: function () {
            expected("an operator");
        }
    };

    type('(regex)', function () {
        return this;
    });

    delim('(endline)');
    delim('(begin)');
    delim('(end)').reach = true;
    delim('(error)').reach = true;
    delim('}').reach = true;
    delim(')');
    delim(']');
    delim('"').reach = true;
    delim("'").reach = true;
    delim(';');
    delim(':').reach = true;
    delim(',');
    reserve('else');
    reserve('case').reach = true;
    reserve('catch');
    reserve('default').reach = true;
    reserve('finally');
//    reservevar('arguments');
    reservevar('eval');
    reservevar('false');
    reservevar('Infinity');
    reservevar('NaN');
    reservevar('null');
    reservevar('this');
    reservevar('true');
    reservevar('undefined');
    assignop('=', 'assign', 20);
    assignop('+=', 'assignadd', 20);
    assignop('-=', 'assignsub', 20);
    assignop('*=', 'assignmult', 20);
    assignop('/=', 'assigndiv', 20).nud = function () {
        error("A regular expression literal can be confused with '/='.");
    };
    assignop('%=', 'assignmod', 20);
    bitwiseassignop('&=', 'assignbitand', 20);
    bitwiseassignop('|=', 'assignbitor', 20);
    bitwiseassignop('^=', 'assignbitxor', 20);
    bitwiseassignop('<<=', 'assignshiftleft', 20);
    bitwiseassignop('>>=', 'assignshiftright', 20);
    bitwiseassignop('>>>=', 'assignshiftrightunsigned', 20);
    infix('?', function (left) {
      this.expr1=left;
      this.expr2=parse(10);
      advance(':');
      this.expr3=parse(10);
      return this;
    }, 30);

    infix('||', 'or', 40);
    infix('&&', 'and', 50);
    bitwise('|', 'bitor', 70);
    bitwise('^', 'bitxor', 80);
    bitwise('&', 'bitand', 90);
    relation('==', function (left, right) {
	this.left=left;
	this.right=right;
	return this;
    });
    relation('===');
    relation('!=', function (left, right) {
	this.left=left;
	this.right=right;
	return this;
    });
    relation('!==');
    relation('<');
    relation('>');
    relation('<=');
    relation('>=');
    bitwise('<<', 'shiftleft', 120);
    bitwise('>>', 'shiftright', 120);
    bitwise('>>>', 'shiftrightunsigned', 120);
    infix('each', 'each', 120);
    infix('in', 'in', 120);
    infix('instanceof', 'instanceof', 120);
    infix('+', 'add', 130);
    prefix('+', 'num');
    infix('-', 'sub', 130);
    prefix('-', 'neg');
    infix('*', 'mult', 140);
    infix('/', 'div', 140);
    infix('%', 'mod', 140);

    suffix('++', 'postinc');
    prefix('++', 'preinc');
    syntax['++'].exps = true;

    suffix('--', 'postdec');
    prefix('--', 'predec');
    syntax['--'].exps = true;
    prefix('delete', function () {
        var p = parse(0);
	this.expr=p;
	return this;
    }).exps = true;


    prefix('~', function () {
	this.expr=parse(150);
	return this;
    });
    prefix('!', 'not');
    prefix('typeof', 'typeof');
    prefix('new', function () {
        var c = parse(155), i;
        if (c) {
            if (c.identifier) {
                c['new'] = true;
            }
        if (c.global)
          c.global.prototype={};//'*':{}};
//print(c.value," ",c.global,"\n");
        }
	var params=[];
        if (nexttoken.id === '(') {
            advance('(');
            if (nexttoken.id !== ')') {
                for (;;) {
                    params.push(parse(10));
                    if (nexttoken.id !== ',') {
                        break;
                    }
                    advance(',');
                }
            }
            advance(')');
        }
	this.func=c;
	this.params=params;
	return this;
    });
    syntax['new'].exps = true;

    infix('.', function (left) {
        var m = identifier();
	if (left.global)
	  note_implied_property(this, left, m);
        this.left = left;
        this.right = m;
        return this;
    }, 160);

    infix('(', function (left) {
        var n = 0;
        var p = [];

        if (nexttoken.id !== ')') {
            for (;;) {
                p[p.length] = parse(10);
                n += 1;
                if (nexttoken.id !== ',') {
                    break;
                }
                advance(',');
            }
        }
        advance(')');
	this.func=left;
	this.params=p;
	return this;
    }, 155).exps = true;

    prefix('(', function () {
        this.expr = parse(0);
        advance(')', this);
	return this;
    });

    infix('[', function (left) {
        var e = parse(0);
        advance(']', this);
	if (left.global)
	  note_implied_property(this, left, e.value);
        this.left = left;
        this.right = e;
	return this;
    }, 160);

    prefix('[', function () {
	this.elements=[];
        if (nexttoken.id === ']') {
            advance(']');
            return this;
        }
        for (;;) {
            this.elements.push(parse(10));
            if (nexttoken.id === ',') {
                advance(',');
            } else {
                advance(']', this);
                return this;
            }
        }
    }, 160);

    (function (x) {
        x.nud = function () {
            var i, s;
            if (nexttoken.id === '}') {
                advance('}');
                return this;
            }
	    this.properties={};
            for (;;) {
                i = optionalidentifier(true);
		var ident;
                if (!i) {
		    ident=nexttoken.value;
                    if (nexttoken.id === '(string)') {
                        i = nexttoken.value;
                        if (ix.test(i)) {
                            s = syntax[i];
                        }
                        advance();
                    } else if (nexttoken.id === '(number)') {
                        i = nexttoken.value.toString();
                        advance();
                    } else {
                        expected("'}'");
                    }
		} else {
		    ident=token.value;
		}
                advance(':');
                this.properties[ident]=parse(10);
                if (nexttoken.id === ',') {
                    advance(',');
                } else {
                    advance('}', this);
                    return this;
                }
            }
        };
        x.fud = function () {
            error("Expected to see a statement and instead saw a block.", token);
        };
    })(delim('{'));

    function varstatement() {

// JavaScript does not have block scope. It only has function scope. So,
// declaring a variable in a block can have unexpected consequences.

        this.vars=[];
        this.init=[];
        for (;;) {
            addlabel(identifier(), 'unused');
            if (nexttoken.id === '=') {
	        var varname = token;
                advance('=');
                if (peek(0).id === '=') {
                    error("Variable {a} was not declared correctly.",
                            nexttoken, nexttoken.value);
                }
		this.vars.push(varname);
		this.init.push(parse(20));
            } else {
	        this.vars.push(token);
	        this.init.push(undefined);
	    }
            if (nexttoken.id !== ',') {
                return this;
            }
            advance(',');
        }
    }


    stmt('var', varstatement);

/*
    stmt('new', function () {
        error("'new' should not be used as a statement.");
    });
*/


    function functionparams() {
        var i, t = nexttoken, p = [];
        advance('(');
        if (nexttoken.id === ')') {
            advance(')');
            return;
        }
        for (;;) {
            i = identifier();
            p.push(i);
            addlabel(i, 'parameter');
            if (nexttoken.id === ',') {
                advance(',');
            } else {
                advance(')', t);
                return p.join(', ');
            }
        }
    }

    function merge(a, b) {
      for (var i in b)
	if (hasOwnProperty.call(b, i)) {
	  if (!hasOwnProperty.call(a, i))
	    a[i]=b[i];
	  else {
	    merge(a[i],b[i]);
	  }
	}
    }

    function doFunction(i) {
        var s = scope;
        scope = object(s);
	var outerimplied=implied;

        funcnest++;
	implied={};

        var ret = {
            '(name)'    : i,
            '(line)'    : nexttoken.line + 1,
            '(context)' : funct,
            '(scope)'   : scope
        };
	funct = ret;
        if (i) {
            addlabel(i, 'function');
	    functions[i]=funct;
        }
        funct['(params)'] = functionparams();
	funct['(body)'] = block(false); //...
        scope = s;
        funct = funct['(context)'];
	if (implied['arguments']) {
	  ret['(selfimplied)']=implied['arguments']['callee'];
	  delete implied['arguments'];
        }

        funcnest--;
        if (funcnest===0)
	  merge(p2implied, implied);
        else
	  merge(outerimplied, implied);
	implied=outerimplied;
	return ret;
    }


    blockstmt('function', function () {
        var i = identifier();
        if (i)
	  addlabel(i, 'unused');
        this.func=doFunction(i);
        if (nexttoken.id === '(' && nexttoken.line === token.line) {
            error(
"Function statements are not invocable. Wrap the function expression in parens.");
        }
	return this;
    });

    prefix('function', function () {
        var i = optionalidentifier();
        this.func=doFunction(i);
	return this;
    });

    blockstmt('if', function () {
        var t = nexttoken;
        advance('(');
	this.expr=parse(0);
        advance(')', t);
        this.ifblock=block(true);
        if (nexttoken.id === 'else') {
            advance('else');
            if (nexttoken.id === 'if' || nexttoken.id === 'switch') {
                this.elseblock=statement();
            } else {
                this.elseblock=block(true);
            }
        }
	
	return this;
    });

    blockstmt('try', function () {
        var b, e, s;
	this.tryblock=block(false);
        if (nexttoken.id === 'catch') {
            advance('catch');
            advance('(');
            s = scope;
            scope = object(s);
            e = nexttoken.value;
            if (nexttoken.type !== '(identifier)') {
            } else {
                addlabel(e, 'unused');
            }
            advance();
	    this.catchident=token;
            advance(')');
            this.catchblock=block(false);
            b = true;
            scope = s;
        }
        if (nexttoken.id === 'finally') {
            advance('finally');
            this.finallyblock=block(false);
	    return this;
        } else if (!b) {
            expected("'catch'");
	}
	return this;
    });

    blockstmt('while', function () {
        var t = nexttoken;
        advance('(');
        this.expr=parse(0);
        advance(')', t);
        this.block=block(true);
	return this;
    }).labelled = true;

    blockstmt('with', function () {
	var outerimplied=implied;

        var t = nexttoken;
        advance('(');
        this.expr=parse(0);
        advance(')');

	if (this.expr.global) {
	  if (!hasOwnProperty.call(this.expr.global, '(with)'))
	    this.expr.global['(with)']={};
	  implied=this.expr.global['(with)'];
	}

        this.block=block(true);

	implied=outerimplied;
	return this;
    }).labelled = true;

    blockstmt('switch', function () {
        var t = nexttoken;
        var g = false;
        advance('(');
        this.condition = parse(20);
        advance(')', t);
        t = nexttoken;
        advance('{');
        this.cases = [];
        for (;;) {
            switch (nexttoken.id) {
            case 'case':
                switch (funct['(verb)']) {
                case 'break':
                case 'case':
                case 'continue':
                case 'return':
                case 'switch':
                case 'throw':
                    break;
                }
                advance('case');
                this.cases.push(parse(20));
                g = true;
                advance(':');
                funct['(verb)'] = 'case';
                break;
            case 'default':
                switch (funct['(verb)']) {
                case 'break':
                case 'continue':
                case 'return':
                case 'throw':
                    break;
                }
                advance('default');
                this.cases.push({id: 'default'});
                g = true;
                advance(':');
                break;
            case '}':
                advance('}', t);
		return this;
            case '(end)':
                error("Missing '{a}'.", nexttoken, '}');
                return;
            default:
                if (g) {
                    switch (token.id) {
                    case ',':
                        error("Each value should have its own case label.");
                        return;
                    case ':':
                        this.cases[this.cases.length-1].statements=statements();
                        break;
                    default:
                        error("Missing ':' on a case clause.", token);
                    }
                } else {
                    expected("'case'");
                }
            }
        }
    }).labelled = true;

    stmt('debugger', function () {
	return this;
    });

    stmt('do', function () {
        this.block=block(true);
        advance('while');
        var t = nexttoken;
        advance('(');
        this.expr=parse(20);
        advance(')', t);
	return this;
    }).labelled = true;

    blockstmt('for', function () {
        var s, t = nexttoken;
        if (nexttoken.id === 'each') {
          this.each=true;
          advance('each');
        }
        advance('(');
        if (peek(nexttoken.id === 'var' ? 1 : 0).id === 'in') {
	    this.forin=true;
            if (nexttoken.id === 'var') {
                advance('var');
                addlabel(identifier(), 'var');
		this.varloopvar=true;
            } else {
                advance();
            }
	    this.loopvar=token;
            advance('in');
            this.expr=parse(20);
            advance(')', t);
            this.block=block(true);
            return this;
        } else {
	    if (nexttoken.id !== ';') {
                if (nexttoken.id === 'var') {
                    advance('var');
                    this.varlist1={};
                    varstatement.call(this.varlist1);
                } else {
		    var p1=[];
                    for (;;) {
                        p1.push(parse(0, 'for'));
                        if (nexttoken.id !== ',') {
	  		    this.list1=p1;
                            break;
                        }
                        advance(',');
                    }
                }
	    }
            advance(';');
            if (nexttoken.id !== ';') {
                this.expr2=parse(20);
            }
            advance(';');
            if (nexttoken.id === ';') {
                expected("')'");
            }
            if (nexttoken.id !== ')') {
	        var p3=[];
                for (;;) {
		    p3.push(parse(0, 'for'));
                    if (nexttoken.id !== ',') {
		        this.list3=p3;
                        break;
                    }
                    advance(',');
                }
            }
            advance(')', t);
            this.block=block(true);
	    return this;
        }
    }).labelled = true;


    stmt('break', function () {
        var v = nexttoken.value;
        if (nexttoken.id !== ';' && nexttoken.id !== '}') {
            advance();
	    this.expr=token;
        }
        reachable('break');
	return this;
    });


    stmt('continue', function () {
        var v = nexttoken.value;
        if (nexttoken.id !== ';') {
            advance();
        }
        reachable('continue');
	return this;
    });


    stmt('return', function () {
        if (nexttoken.id !== ';' && !nexttoken.reach) {
            this.expr=parse(20);
        }
        reachable('return');
	return this;
    });


    stmt('throw', function () {
        this.expr=parse(20);
        reachable('throw');
	return this;
    });


//  Superfluous reserved words

    reserve('abstract');
    reserve('boolean');
    reserve('byte');
    reserve('char');
    reserve('class');
    reserve('const');
    reserve('double');
    reserve('enum');
    reserve('export');
    reserve('extends');
    reserve('final');
    reserve('float');
    reserve('goto');
    reserve('implements');
    reserve('import');
    reserve('int');
    reserve('interface');
    reserve('long');
    reserve('native');
    reserve('package');
    reserve('private');
    reserve('protected');
    reserve('public');
    reserve('short');
    reserve('static');
    reserve('super');
    reserve('synchronized');
    reserve('throws');
    reserve('transient');
    reserve('void');
    reserve('volatile');


// The actual JSLINT function itself.

    var JSLINT = function (s, _filename, o) {
        if (o) {
            option = o;
        } else {
            option = {};
        }
	filename=_filename;
        globals = object(standard);
        JSLINT.errors = [];
        global = object(globals);
        scope = global;
        funct = {'(global)': true, '(name)': '(global)', '(scope)': scope};
        functions = {};
        src = false;
        stack = null;
        implied = {};
        p2implied = {};
        funcnest = 0;
        inblock = false;
        lookahead = [];
        lex.init(s);
        prereg = true;

        prevtoken = token = nexttoken = syntax['(begin)'];

	advance();
	var ret={
	  statements: statements(),
	  functions: functions,
	  p1implied: implied,
	  p2implied: p2implied
	}
	advance('(end)');

	return ret;
    };

    return JSLINT;

})()
