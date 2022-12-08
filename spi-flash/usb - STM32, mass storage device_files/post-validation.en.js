(()=>{var e={p:""};(()=>{"use strict";e.p=document.getElementById("webpack-public-path").innerText+"Js/"})(),StackExchange=window.StackExchange=window.StackExchange||{},StackOverflow=window.StackOverflow=window.StackOverflow||{},StackExchange.postValidation=function(){var e=$("body").hasClass("js-ask-page-v2"),t=$("body").hasClass("js-staging-ground-wizard"),n="Title",s="Body",i="Tags",a="Mentions",r="EditComment",o="Excerpt",l="Email",c="General";const u="disable-blur-validation";function d(e,t,n){var s={Title:".js-post-title-field",Body:".js-post-body-field[data-post-type-id="+t+"]",Tags:".js-post-tags-field",Mentions:".js-post-mentions-field",EditComment:".js-post-edit-comment-field",Excerpt:".js-post-excerpt-field",Email:".js-post-email-field",ArticleType:".js-article-type-field",Date:".js-post-date-field",CommentForm:".js-comment-text-input",Subtitle:".js-post-subtitle-field"};return s[n]?e.find(s[n]):$()}function g(e,t,n){var s=d(e,t,n);return n===i||n===a?e.find(".js-tag-editor").filter((function(){return $(this).data("target-field")===s.get(0)})):s}var b=[];function f(t,a,c,u){var d=t.find('input[type="submit"]:visible, button[type="submit"]:visible'),g=d.length&&d.is(":enabled");g&&d.prop("disabled",!0),function(t,s,i){v(t,s,i,n,(function(t,n,s){var i=t.val(),a=$.trim(i).length,r=t.data("min-length"),o=t.data("max-length");0!==a||e?r&&a<r?n(__tr(["Title must be at least $minLength$ character.","Title must be at least $minLength$ characters."], {minLength: r}, "en", ["minLength"])):o&&a>o?n(__tr(["Title cannot be longer than $maxLength$ character.","Title cannot be longer than $maxLength$ characters."], {maxLength: o}, "en", ["maxLength"])):s({type:"POST",url:"/posts/validate-title",data:{title:i,fkey:StackExchange.options.user.fkey}}):n()}))}(t,a,c),function(t,n,i,a){v(t,n,i,s,(function(t,s,i){var r=t.val(),o=$.trim(r).length,l=t.data("min-length");0!==o||e?5!==n?14!==n?1!==n&&2!==n||i({type:"POST",url:"/posts/validate-body",data:{body:r,oldBody:t.prop("defaultValue"),isQuestion:1===n,isSuggestedEdit:a,fkey:StackExchange.options.user.fkey}}):l&&o<l?s(__tr(["Body must be at least $minLength$ characters."], {minLength: l}, "en", [])):s():l&&o<l?s(__tr(["Wiki Body must be at least $minLength$ characters. You entered $actual$."], {minLength: l,actual: o}, "en", [])):s():s()}))}(t,a,c,u),function(e,t,n){v(e,t,n,r,(function(s,i,a){var r=s.val(),o=$.trim(r).length,l=s.data("min-length"),c=s.data("max-length");0!==o?l&&o<l?i(__tr(["Your edit summary must be at least $minLength$ character.","Your edit summary must be at least $minLength$ characters."], {minLength: l}, "en", ["minLength"])):c&&o>c?i(__tr(["Your edit summary cannot be longer than $maxLength$ character.","Your edit summary cannot be longer than $maxLength$ characters."], {maxLength: c}, "en", ["maxLength"])):P(e,t,n)||i():i()}))}(t,a,c),function(e,t,n){v(e,t,n,o,(function(e,t,n){var s=e.val(),i=$.trim(s).length,a=e.data("min-length"),r=e.data("max-length");0!==i?a&&i<a?t(__tr(["Wiki Excerpt must be at least $minLength$ characters; you entered $actual$."], {minLength: a,actual: i}, "en", [])):r&&i>r?t(__tr(["Wiki Excerpt cannot be longer than $maxLength$ characters; you entered $actual$."], {maxLength: r,actual: i}, "en", [])):t():t()}))}(t,a,c),function(e,t,n){v(e,t,n,l,(function(e,t,n){var s=e.val(),i=$.trim(s);0!==i.length?StackExchange.helpers.isEmailAddress(i)?t():t(__tr(["This email does not appear to be valid."], undefined, "en", [])):t()}))}(t,a,c),w(t,a,(function(){!function(t,n,s){v(t,n,s,i,(function(t,n,s,i){var a=t.val();0!==$.trim(a).length||e?s({type:"POST",url:"/posts/validate-tags",data:{tags:a,oldTags:t.prop("defaultValue"),fkey:StackExchange.options.user.fkey,postTypeId:i},success:function(e){var n=t.closest(".js-post-form").find(".js-warned-tags-field");if(n.length){var s=n.val(),i=n.data("warned-tags")||[],a=((e.source||{}).Tags||[]).filter((function(e){return e&&-1===i.indexOf(e)}));a.length>0&&StackExchange.using("gps",(function(){a.forEach((function(e){StackExchange.gps.track("tag_warning.show",{tag:e},!0),s+=" "+e,i.push(e)})),n.val($.trim(s)).data("warned-tags",i),StackExchange.gps.sendPending()}))}}}):n()}))}(t,a,c),g&&d.prop("disabled",!1)}))}function p(e,t){e.find('input[type="submit"]:visible, button[type="submit"]').removeClass("is-loading"),t||(StackExchange.helpers.enableSubmitButton(e),StackExchange.navPrevention&&StackExchange.navPrevention.start())}function h(e,t,n,s,i){$.ajax({type:"POST",dataType:"json",data:e.serialize(),url:e.attr("action"),success:i,error:function(){var s=y(n,0);A(e,t,n,{General:[$("<span/>").text(s).html()]},0)},complete:function(){p(e,s)}})}function m(){for(var e=0;e<b.length;e++)clearTimeout(b[e]);b=[]}function v(t,n,s,i,a){d(t,n,i).blur((function(){var r=this,o=$(this);if(!o.data(u)){var l=function(e){S(t,n,s,i,e)},c=function(e){return k(e,t,n,s,[i])};b.push(setTimeout((function(){var t=StackExchange.stacksValidation.handlerFor(o);t&&!e&&t.clear(),a.call(r,o,l,c,n)}),250))}}))}function P(e,t,n){return"[Edit removed during grace period]"===$.trim(d(e,t,r).val())&&(S(e,t,n,r,__tr(["Comment reserved for system use. Please use an appropriate comment."], undefined, "en", [])),!0)}function y(e,t){if(t>0)switch(e){case"question":return __tr(["Your question couldn't be submitted. Please see the error above.","Your question couldn't be submitted. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"]);case"answer":return __tr(["Your answer couldn't be submitted. Please see the error above.","Your answer couldn't be submitted. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"]);case"edit":return __tr(["Your edit couldn't be submitted. Please see the error above.","Your edit couldn't be submitted. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"]);case"tags":return __tr(["Your tags couldn't be submitted. Please see the error above.","Your tags couldn't be submitted. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"]);case"article":return __tr(["Your article couldn't be submitted. Please see the errors above.","Your article couldn't be submitted. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"]);case"announcement":return __tr(["Your bulletin couldn't be published. Please see the errors above.","Your bulletin couldn't be published. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"]);default:return __tr(["Your post couldn't be submitted. Please see the error above.","Your post couldn't be submitted. Please see the errors above."], {specificErrorCount: t}, "en", ["specificErrorCount"])}else switch(e){case"question":return __tr(["An error occurred submitting the question."], undefined, "en", []);case"answer":return __tr(["An error occurred submitting the answer."], undefined, "en", []);case"edit":return __tr(["An error occurred submitting the edit."], undefined, "en", []);case"tags":return __tr(["An error occurred submitting the tags."], undefined, "en", []);case"article":return __tr(["An error occurred submitting the article."], undefined, "en", []);case"announcement":return __tr(["An error occurred publishing the bulletin."], undefined, "en", []);default:return __tr(["An error occurred submitting the post."], undefined, "en", [])}}function A(e,t,n,s,i){var a=e.find(".js-general-error").text("").removeClass("d-none");T(e,a,s,null,c,t,n)||(i>0?a.text(y(n,i)):a.addClass("d-none"))}function E(e){var t=$(".js-post-review-summary").closest(".js-post-review-summary-container");if(t.length>0)t.filter(":visible").scrollIntoView();else{var n,s;C()&&($("#sidebar").animate({opacity:.4},500),n=setInterval((function(){C()||($("#sidebar").animate({opacity:1},500),clearInterval(n))}),500)),e.find(".validation-error, .js-stacks-validation.has-error").each((function(){var e=$(this).offset().top;(!s||e<s)&&(s=e)}));var i=function(){for(var t=0;t<3;t++)e.find(".message").animate({left:"+=5px"},100).animate({left:"-=5px"},100)};if(s){var a=$(".review-bar").length;s=Math.max(0,s-(a?125:30)),$("html, body").animate({scrollTop:s},i)}else i()}}function x(e,t,c,u,d){if(!u)return;const g=e.add("#js-comments-container");w(e,t,(function(){var e=z(g,t,c,[n,s,i,a,r,o,l,"ArticleType","Date","CommentForm","Subtitle"],u,d).length;A(g,t,c,u,e),E(g)}))}function w(e,t,n){var s=function(){1!==t||g(e,t,i).length?n():setTimeout(s,250)};s()}function k(e,t,n,s,i,a){return $.ajax(e).then((function(e){return a?$.when(a()).then((function(){return e})):e})).done((function(e){z(t,n,s,i,e.errors,e.warnings)})).fail((function(){z(t,n,s,i,{},{})}))}function z(e,t,n,s,i,a){for(var r=[],o=0;o<s.length;o++){var l=s[o];T(e,g(e,t,l),i,a,l,t,n)&&r.push(l)}return r}function S(e,t,n,s,i){j(e,g(e,t,s),i?[$("<span/>").text(i).html()]:[],[],s,t,n)}function T(e,t,n,s,i,a,r){return j(e,t,n[i]||[],(s||{})[i]||[],i,a,r)}function j(t,n,s,i,a,r,o){var l=StackExchange.stacksValidation.handlerFor(n);return l?function(t,n,s,i,a){t.clear("error"),i.forEach((function(e){t.add("error",e)})),"edit"===s||"question"===s&&e||(t.clear("warning"),a.forEach((function(e){t.add("warning",e)})))}(l,0,o,s,i):function(e,t,n){e&&e.length&&(0===n.length||1===n.length&&""===n[0]||!$("html").has(e).length?function(e){var t=e.data("error-popup");t&&t.is(":visible")&&t.fadeOutAndRemove(),e.removeClass("validation-error"),e.removeData("error-popup"),e.removeData("error-message")}(e):function(e,t,n){var s=1===t.length?t[0]:"<ul><li>"+t.join("</li><li>")+"</li></ul>",i=e.data("error-popup");if(i&&i.is(":visible")){if(e.data("error-message")===s)return void(i.animateOffsetTop&&i.animateOffsetTop(0));i.fadeOutAndRemove()}var a=StackExchange.helpers.showMessage(e,s,n);a.find("a").attr("target","_blank"),a.click(m),e.addClass("validation-error").data("error-popup",a).data("error-message",s)}(e,n,function(e,t){var n=$("#sidebar, .sidebar").first().width()||270,s="lg"===StackExchange.responsive.currentRange();return e===c?{position:"inline",css:{display:"inline-block","margin-bottom":"10px"},closeOthers:!1,dismissable:!1,type:t}:{position:{my:s?"left top":"top center",at:s?"right center":"bottom center"},css:{"max-width":n,"min-width":n},closeOthers:!1,type:t}}(t,"error")))}(n,a,s),s.length&&d(t,r,a).data(u,!0).one("input change",(function(){$(this).data(u,null)})),t.find(".validation-error, .js-stacks-validation.has-error").length||t.find(".js-general-error").text(""),n.trigger("post:validated-field",[{errors:s,warnings:i,field:a,postTypeId:r,formType:o}]),s.length>0}function C(){var e=!1,t=$("#sidebar, .sidebar").first();if(!t.length)return!1;var n=t.offset().left;return $(".message").each((function(){var t=$(this);if(t.offset().left+t.outerWidth()>n)return e=!0,!1})),e}return{initOnBlur:f,initOnBlurAndSubmit:function(e,t,n,s,a){var r;f(e,t,n,s);var o=function(s){if(e.trigger("post:submit-completed",[{formType:n,postTypeId:t,response:s}]),s.success)if(a)a(s);else{var i=window.location.href.split("#")[0],l=s.redirectTo.split("#")[0];0===l.indexOf("/")&&(l=window.location.protocol+"//"+window.location.hostname+l),window.onbeforeunload=null,r=!0,window.location=s.redirectTo,i.toLowerCase()===l.toLowerCase()&&window.location.reload(!0)}else s.captchaHtml?StackExchange.nocaptcha.init(s.captchaHtml,o):s.errors?(e.find(".js-post-prior-attempt-count").val((function(e,t){return(+t+1||0).toString()})),x(e,t,n,s.errors,s.warnings)):A(e,t,n,{General:[$("<span/>").text(s.message).html()]},0)};e.submit((function(){if(e.find(".js-post-answer-while-asking-checkbox").is(":checked"))return!0;if(P(e,t,n))return StackExchange.helpers.enableSubmitButton(e),!1;if(m(),StackExchange.navPrevention&&StackExchange.navPrevention.stop(),e.find('input[type="submit"]:visible, button[type="submit"]').addClass("is-loading"),StackExchange.helpers.disableSubmitButton(e),StackExchange.options.site.enableNewTagCreationWarning&&14!=t){var s=d(e,t,i),a=s.prop("defaultValue");if(s.val()!==a)return $.ajax({type:"GET",url:"/posts/new-tags-warning",dataType:"json",data:{tags:s.val()},success:function(s){if(s.showWarning){var a={closeOthers:!0,shown:function(){$(".js-confirm-tag-creation").on("click",(function(s){return StackExchange.helpers.closePopups(),h(e,t,n,r,o),s.preventDefault(),!1}))},dismissing:function(){p(e,r)},returnElements:g(e,t,i).find("input:visible")};StackExchange.helpers.showModal($(s.html).elementNodesOnly(),a),StackExchange.helpers.bindMovablePopups()}else h(e,t,n,r,o)}}),!1}return setTimeout((function(){h(e,t,n,r,o)}),0),!1}))},showErrorsAfterSubmission:x,validatePostFields:function(e,a,r,o,l){if(1===a)return k({type:"POST",url:"/posts/validate-question",data:{title:d(e,a,n).val(),body:d(e,a,s).val(),tags:d(e,a,i).val(),fkey:StackExchange.options.user.fkey,isAskWizard:t}},e,a,r,[n,s,i],l).promise();if(2===a)return k({type:"POST",url:"/posts/validate-body",data:{body:d(e,a,s).val(),oldBody:d(e,a,s).prop("defaultValue"),isQuestion:!1,isSuggestedEdit:o||!1,fkey:StackExchange.options.user.fkey}},e,a,r,[s],l).promise();var c=$.Deferred();return c.reject(),c.promise()},scrollToErrors:E}}()})();