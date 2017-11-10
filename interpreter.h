#ifndef V8TEMPLATE_INTERPRETER_H
#define V8TEMPLATE_INTERPRETER_H

#include <string>
#include <v8.h>

/* php-like templating system using javascript

<div><?=name?></div>
<? if(birthdate) { ?>
Born on: <?=birthdate?>
<? } ?>

function() {
var str = ""';
str += "<div>"
str += name;
str += "</div>";
if(typeof(birthdate) != 'undefined') {
  str += "Born on: ";
  str += birthdate;

Remember to set engine before loading from file

*/

namespace v8template {

class Interpreter
{
public:
	Interpreter();
	virtual ~Interpreter();

	void assign(const std::string& name, const std::string& value);
	void assign(
		const std::string& name,
		v8::Handle<v8::Value> (*func)(const v8::Arguments& args));

	std::string run(const std::string& tmpl);

protected:
	std::string preprocess(const std::string& tpl);

	// Create a stack-allocated handle scope.
	v8::Persistent<v8::Context> _context;
};

}

#endif // V8TEMPLATE_INTERPRETER_H
