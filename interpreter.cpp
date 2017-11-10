#include "interpreter.h"
#include <assert.h>

namespace v8template {


static std::string replace(
	const std::string& pattern, const std::string& replacement, std::string& s)
{
	size_t p = s.find(pattern);

	for (size_t p = s.find(pattern); p != std::string::npos; p = s.find(pattern)) {
		s.replace(p, pattern.length(), replacement);
	}

	return s;
}

static std::string escape(std::string s)
{
	replace("\\", "\\\\", s);
	replace("\'", "\\\'", s);
	return s;
}


Interpreter::Interpreter():
	_context(v8::Context::New()) // Create a new _context.
{
	// Enter the created _context for compiling and
	// running the hello world program.
	_context->Enter();
}

Interpreter::~Interpreter() {
	// Get rid of the persistent _context.
	v8::Isolate* isolate = _context->GetIsolate();
	_context.Dispose(isolate);
}


void Interpreter::assign(const std::string& name, const std::string& value)
{
	v8::HandleScope scope;
	v8::Local<v8::Value> n = v8::String::New(name.c_str());
	v8::Local<v8::Value> v = v8::String::New(value.c_str());
	_context->Global()->Set(n, v);
}

void Interpreter::assign(const std::string& name, v8::Handle<v8::Value> (*func)(const v8::Arguments& args))
{
	v8::HandleScope scope;
	_context->Global()->Set(
		v8::String::NewSymbol(name.c_str()),
		v8::FunctionTemplate::New(func)->GetFunction()
	);
}

std::string Interpreter::run(const std::string& tmpl)
{
	v8::HandleScope scope;
	// Create a string containing the JavaScript code
	// to execute (notice the quotation).

	std::string js = preprocess(tmpl);
	v8::Local<v8::String> source = v8::String::New(js.c_str());

	// Compile the Javascript code.
	v8::Local<v8::Script> script = v8::Script::Compile(source);

	// Run the script to get the result.
	v8::Local<v8::Value> result = script->Run();
	v8::String::Utf8Value utf8(result);

	return *utf8;
}

std::string Interpreter::preprocess(const std::string& tpl) {
	/* We build a simple tokenizer (can't use bison, damn windows)
		state: outside or inside
		outside:
		look for <?
		s += "[text]";
		go inside;
		inside:
		look for ?>
		if first char is = s += [text];
		else [text];
		go outside
	*/
	std::string in("<?");
	std::string out("?>");
	int position = 0;
	int next = 0;
	std::string str = "(function() { this._out = '';\n";
	bool inside = false;

	while (position < tpl.size()) {
		if (!inside) {
			next = tpl.find(in, position);
			if (next == std::string::npos) next = tpl.size();

			std::string escaped = escape(tpl.substr(position, next - position));
			replace("\n", "\\n", escaped);

			if (escaped != "") {
				str += "this._out += '" + escaped + "';\n";
			}

			position = next + 2;
			inside = true;

		} else {
			next = tpl.find(out, position);
			assert(next != std::string::npos);

			bool shortform = (tpl[position] == '=');

			if (shortform) {
				position++;
				const std::string& exp = tpl.substr(position, next - position);
				str += "this._out += typeof " + exp + " === 'undefined'? '': "+ exp + ";\n";

			} else {
				str += tpl.substr(position, next - position); + "\n";
			}

			position = next + 2;

			inside = false;
		}
	}

	str += " return this._out; })();";
// 		std::cout << str << std::endl;

	return str;
}

}
