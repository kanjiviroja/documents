// This file was automatically generated from headerBadgeByline.soy.
// Please don't edit this file by hand.

goog.provide('jive.unified.content.view.headerBadgeByline');

goog.require('soy');
goog.require('soydata');
goog.require('soy.StringBuilder');
goog.require('jive.shared.displayutil.userDisplayNameLink');


jive.unified.content.view.headerBadgeByline = function(opt_data, opt_sb) {
  var output = opt_sb || new soy.StringBuilder();
  output.append('<span class="j-outcome-byline font-color-meta">', soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg(jive.i18n.getMinKey('outcomes.badge.popup.' + opt_data.outcomeTypeName)),[])), ' ');
  jive.shared.displayutil.userDisplayNameLink(soy.$$augmentMap(opt_data.author, {anonymous: false}), output);
  output.append(' <span class="font-color-meta">', soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k28f9'),[((opt_data.modifiedDate ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.modifiedDate) : '') ? require('moment')((parseFloat((opt_data.modifiedDate ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.modifiedDate) : '')) ? parseFloat((opt_data.modifiedDate ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.modifiedDate) : '')) : (opt_data.modifiedDate ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.modifiedDate) : ''))).locale(_jive_locale.toLowerCase().replace('_','-')).format('lll') : '')])), '</span></span>');
  return opt_sb ? '' : output.toString();
};
