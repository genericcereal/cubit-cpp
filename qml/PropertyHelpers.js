.pragma library

// Element type checks
function isFrame(el) {
    return el && el.elementType === "Frame"
}

function isFrameVariant(el) {
    return el && el.elementType === "FrameComponentVariant"
}

function isInstance(el) {
    return el && el.elementType === "FrameComponentInstance"
}

function isText(el) {
    return el && el.elementType === "Text"
}

function isTextVariant(el) {
    return el && el.elementType === "TextComponentVariant"
}

function isVariable(el) {
    return el && el.elementType === "Variable"
}

function isDesignElement(el) {
    return el && el.isDesignElement
}

function isVisual(el) {
    return el && el.isVisual
}

// Property visibility logic
function canShow(prop, el, editableProps) {
    if (!el) return false
    
    if (isInstance(el)) {
        return editableProps && editableProps.indexOf(prop) !== -1
    }
    
    return isFrame(el) || isFrameVariant(el) || isVariable(el)
}

// Specific property visibility helpers
function canShowFill(el, editableProps) {
    return canShow("fill", el, editableProps)
}

function canShowOverflow(el, editableProps) {
    return canShow("overflow", el, editableProps)
}

function canShowBorderRadius(el, editableProps) {
    return canShow("borderRadius", el, editableProps)
}

function canShowBorderWidth(el, editableProps) {
    return canShow("borderWidth", el, editableProps)
}

function canShowBorderColor(el, editableProps) {
    return canShow("borderColor", el, editableProps)
}

function canShowPlatform(el, editableProps, application) {
    return canShow("platform", el, editableProps) && application && application.activeCanvas && application.activeCanvas.platforms.length > 0
}

function canShowRole(el, editableProps) {
    return canShow("role", el, editableProps) && el && el.platform && el.platform !== "" && !isVariable(el)
}

function canShowFlex(el, editableProps) {
    return canShow("flex", el, editableProps) && !isVariable(el)
}

function canShowOrientation(el, editableProps) {
    return canShow("orientation", el, editableProps) && !isVariable(el)
}

function canShowGap(el, editableProps) {
    return canShow("gap", el, editableProps) && !isVariable(el)
}

function canShowJustify(el, editableProps) {
    return canShow("justify", el, editableProps) && !isVariable(el)
}

function canShowAlign(el, editableProps) {
    return canShow("align", el, editableProps) && !isVariable(el)
}

// Element visibility checks
function showPositionSize(el) {
    return el && isVisual(el) && !(isDesignElement(el) && el.parentId)
}

function showPosition(el) {
    return el && isDesignElement(el) && el.parentId
}

function showSize(el) {
    return el && isDesignElement(el) && el.parentId
}

function showFrameStyle(el) {
    return el && (isFrame(el) || isFrameVariant(el) || (isInstance(el) && el.hasOwnProperty("fill")))
}

function showText(el) {
    return el && (isText(el) || isTextVariant(el) || (isInstance(el) && el.hasOwnProperty("content")))
}

function showVariable(el) {
    return el && isVariable(el)
}

function showFrameInVariant(el, application) {
    if (!el || !isFrame(el) || !el.parentId) {
        return false
    }
    if (!application || !application.activeCanvas) return false
    var model = application.activeCanvas.elementModel
    if (!model) return false
    
    var parentElement = model.getElementById(el.parentId)
    return parentElement && parentElement.elementType === "ComponentVariant"
}

function showVariant(el) {
    return el && isFrameVariant(el)
}

function showActions(el) {
    return el && isDesignElement(el)
}

function showPlatforms(el) {
    return !el
}