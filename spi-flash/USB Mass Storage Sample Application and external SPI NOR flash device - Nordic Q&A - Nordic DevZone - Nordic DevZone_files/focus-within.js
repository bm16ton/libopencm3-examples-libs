// https://raw.githubusercontent.com/jonathantneal/focus-within/master/src/browser.js
"use strict";

function focusWithin(document, opts) {
    var className = Object(opts).className;
    var attr = Object(opts).attr || 'focus-within';
    var force = Object(opts).force;
    var lastElements = [];

    try {
        document.querySelector(':focus-within');

        if (!force) {
            return initialize;
        }
    } catch (ignoredError) {
        /* do nothing and continue */
    }

    function onfocuschange() {
        var lastElement;

        while (lastElement = lastElements.pop()) {
            if (attr) {
                lastElement.removeAttribute(attr);
            }

            if (className) {
                lastElement.classList.remove(className);
            }
        }

        var activeElement = document.activeElement; // only add focus if it has not been added and is not the document element

        if (!/^(#document|HTML|BODY)$/.test(Object(activeElement).nodeName)) {
            while (activeElement && activeElement.nodeType === 1) {
                if (attr) {
                    activeElement.setAttribute(attr, '');
                }

                if (className) {
                    activeElement.classList.add(className);
                }

                lastElements.push(activeElement);
                activeElement = activeElement.parentNode;
            }
        }
    }

    function initialize() {
        document.addEventListener('focus', onfocuschange, true);
        document.addEventListener('blur', onfocuschange, true);
    }
    /**
     * Callback wrapper for check loaded state
     */

    /* eslint-disable */


    !function load() {
        if (/m/.test(document.readyState)) {
            document.removeEventListener('readystatechange', load) | initialize();
        } else {
            document.addEventListener('readystatechange', load);
        }
    }();
    /* eslint-enable */

    return initialize;
}
