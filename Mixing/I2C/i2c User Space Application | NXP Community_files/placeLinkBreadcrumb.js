// This file was automatically generated from placeLinkBreadcrumb.soy.
// Please don't edit this file by hand.

goog.provide('jive.shared.breadcrumb.placeLinkBreadcrumb');

goog.require('soy');
goog.require('soydata');
goog.require('soy.StringBuilder');
goog.require('jive.shared.breadcrumb.placeLinkPopover');


jive.shared.breadcrumb.placeLinkBreadcrumb = function(opt_data, opt_sb) {
  var output = opt_sb || new soy.StringBuilder();
  if (opt_data.placeLinkCount && opt_data.placeLinkCount > 0 && opt_data.linkedContentType && opt_data.linkedContentID) {
    output.append((opt_data.legacy) ? ((opt_data.place || opt_data.parents) ? ' ' + soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k20a3'),[])) + ' ' : '') + '<a href="#" class="js-place-linked-content-link">' + ((opt_data.placeLinkCount == 1) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1700'),[opt_data.placeLinkCount])) : (opt_data.placeLinkCount == 2) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1702'),[opt_data.placeLinkCount])) : (opt_data.placeLinkCount == 3) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1701'),[opt_data.placeLinkCount])) : soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k16ff'),[opt_data.placeLinkCount]))) + '</a>' : (opt_data.userContainer) ? '<span>' + ((opt_data.linkedContentType != 38) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k20a3'),[])) : '') + ' <a href="#" class="js-place-linked-content-link">' + ((opt_data.placeLinkCount == 1) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1714'),[opt_data.placeLinkCount])) : (opt_data.placeLinkCount == 2) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1716'),[opt_data.placeLinkCount])) : (opt_data.placeLinkCount == 3) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1715'),[opt_data.placeLinkCount])) : soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1713'),[opt_data.placeLinkCount]))) + '</a>' + soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k20a0'),[])) + '</span>' : '<span> ' + soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k16fe'),[])) + ' <a href="#" class="js-place-linked-content-link">' + ((opt_data.placeLinkCount == 1) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k170b'),[opt_data.placeLinkCount])) : (opt_data.placeLinkCount == 2) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1712'),[opt_data.placeLinkCount])) : (opt_data.placeLinkCount == 3) ? soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1711'),[opt_data.placeLinkCount])) : soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1703'),[opt_data.placeLinkCount]))) + '</a></span>');
    if (opt_data.renderPopoverContainer) {
      jive.shared.breadcrumb.placeLinkPopover(null, output);
    }
  }
  return opt_sb ? '' : output.toString();
};
