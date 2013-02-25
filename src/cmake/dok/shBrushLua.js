SyntaxHighlighter.brushes.Lua = function()
{
	// Copyright 2008 HyukYi, Kwon : http://www.nekothink.com
	var funcs = 
	'_VERSION assert collectgarbage dofile error gcinfo loadfile loadstring print ' +
	'tonumber tostring type unpack _ALERT _ERRORMESSAGE _INPUT _PROMPT _OUTPUT ' +
	'_STDERR _STDIN _STDOUT call dostring foreach foreachi getn globals newtype ' +
	'rawget rawset require sort tinsert tremove _G getfenv getmetatable ipairs ' +
	'loadlib next pairs pcall rawegal rawget rawset require setfenv setmetatable ' +
	'xpcall string table math coroutine io os debug ' +
	
	'abs acos asin atan atan2 ceil cos deg exp floor format frexp gsub ldexp log ' +
	'log10 max min mod rad random randomseed sin sqrt strbyte strchar strfind strlen ' +
	'strlower strrep strsub strupper tan string.byte string.char string.dump ' +
	'string.find string.len string.lower string.rep string.sub string.upper ' +
	'string.format string.gfind string.gsub table.concat table.foreach ' +
	'table.foreachi table.getn table.sort table.insert table.remove table.setn ' +
	'math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.deg ' +
	'math.exp math.floor math.frexp math.ldexp math.log math.log10 math.max math.min ' +
	'math.mod math.pi math.rad math.random math.randomseed math.sin math.sqrt math.tan ' +
	
	'openfile closefile readfrom writeto appendto remove rename flush seek tmpfile ' +
	'tmpname read write clock date difftime execute exit getenv setlocale time ' +
	'coroutine.create coroutine.resume coroutine.status coroutine.wrap ' +
	'coroutine.yield io.close io.flush io.input io.lines io.open io.output io.read ' +
	'io.tmpfile io.type io.write io.stdin io.stdout io.stderr os.clock os.date ' +
	'os.difftime os.execute os.exit os.getenv os.remove os.rename os.setlocale ' +
	'os.time os.tmpname';

	// Copyright 2008 HyukYi, Kwon : http://www.nekothink.com
	var keywords = 
	'and break do else elseif end false for function if in local nil not or repeat ' +
	'return then true until while';
	
	// Copyright 2009 ikpil, Choi : http://www.ikpil.com
	this.regexList = [
		{ regex: new RegExp('--\\[\\[[\\s\\S]*\\]\\]--', 'gmi'),			css: 'comments' },
		{ regex: new RegExp('--[^\\[]{2}.*$', 'gmi'),						css: 'comments' },
		{ regex: SyntaxHighlighter.regexLib.multiLineDoubleQuotedString,	css: 'string' },
		{ regex: new RegExp(this.getKeywords(funcs), 'gmi'),				css: 'functions bold' },		
		{ regex: new RegExp(this.getKeywords(keywords), 'gmi'),				css: 'keyword bold' }
		];

};
SyntaxHighlighter.brushes.Lua.prototype	= new SyntaxHighlighter.Highlighter();
SyntaxHighlighter.brushes.Lua.aliases	= ['lua'];

