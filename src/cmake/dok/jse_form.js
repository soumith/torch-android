
// ---------- script properties ----------

var bold = 0;


// ---------- load pages here ----------

var s = new Array();

// SEARCH_ARRAY //


// ---------- end of script properties and sites ----------
var od;
var co;
var r;
var m;
function dosearch(d) {
    od = d;
    m = 0;
    if (d.charAt(0) == '"' && d.charAt(d.length - 1) == '"') {
        m = 1;
    }
    r = new Array();
    co = 0
    if (m == 0) {
        var woin = new Array();
        var w = d.split(" ");
        for (var a = 0; a < w.length; a++) {
            woin[a] = 0;
            if (w[a].charAt(0) == '-') {
                woin[a] = 1;
            }
        }
        for (var a = 0; a < w.length; a++) {
            w[a] = w[a].replace(/^~^\-|^~^\+/gi, "");
        }
        a = 0;
        for (var c = 0; c < s.length; c++) {
            pa = 0;
            nh = 0;
            for (var i = 0; i < woin.length; i++) {
                if (woin[i] == 0) {
                    nh++;
                    var pat = new RegExp(w[i], "i");
                    var rn = s[c].search(pat);
                    if (rn >= 0) {
                        pa++;
                    } else {
                        pa = 0;
                    }
                }
                if (woin[i] == 1) {
                    var pat = new RegExp(w[i], "i");
                    var rn = s[c].search(pat);
                    if (rn >= 0) {
                        pa = 0;
                    }
                }
            }
            if (pa == nh) {
                r[a] = s[c];
            a++;
            }
        }
        co = a;
    }
    
    if (m == 1) {
        d = d.replace(/"/gi, "");
        var a = 0;
        var pat = new RegExp(d, "i");
        for (var c = 0; c < s.length; c++) {
            var rn = s[c].search(pat);
            if (rn >= 0) {
                r[a] = s[c];
                a++;
            }
        }
        co = a;
    }
}

function out_jse() {
    var res = '';
    if (co == 0) {
        res = 'Your search did not match any documents.<p>Make sure all keywords are spelt correctly.<br>Try different or more general keywords.<p>(Also, run this once to make sure help is searchable: torch -e "dok.installsearch()")';
        return res;
    }
    for (var a = 0; a < r.length; a++) {
        var os = r[a].split("^~^");
        if (bold == 1 && m == 1) {
            var br = "<b>" + d + "</b>";
            os[2] = os[2].replace(pat, br);
        }
        var num = String(a + 1);
        res = res + '<h3>' + num + '. <a href="' + os[1] + '" class="anchor">' + os[0] + '</a></h3>' + os[2] + '<p>';
    }
    return res;
}

function search_form(jse_Form) {
    if (jse_Form.d.value.length > 0) {
        dosearch(jse_Form.d.value);
        var res = co + ' page(s) found<p>';
        res = res + out_jse();
        $("#results").html(res);
        $('#contents').hide();
        $('#toc').hide();
    }
}
