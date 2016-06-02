// This file was automatically generated from correctAnswer.soy.
// Please don't edit this file by hand.

goog.provide('jive.question.view.correctAnswer');

goog.require('soy');
goog.require('soydata');
goog.require('soy.StringBuilder');
goog.require('jive.shared.displayutil.coreApiAvatar');
goog.require('jive.shared.displayutil.coreApiUserDisplayNameLink');


jive.question.view.correctAnswer = function(opt_data, opt_sb) {
  opt_data = opt_data || {};
  var output = opt_sb || new soy.StringBuilder();
  if (opt_data.correctAnswer) {
    output.append('<div class="j-question-content-footer js-body-answer j-answer-correct clearfix"><div class="reply jive-comment-content clearfix">');
    jive.shared.displayutil.coreApiAvatar(soy.$$augmentMap(opt_data.correctAnswer.author, {size: 72, currentUserPartner: opt_data.currentUserPartner}), output);
    output.append('<div class="j-comment-header"><span class="jive-comment-meta font-color-meta-light"><span class="j-username-wrap">');
    jive.shared.displayutil.coreApiUserDisplayNameLink(opt_data.correctAnswer.author, output);
    output.append('  </span><a href="#comment-', soy.$$escapeHtmlAttribute(opt_data.correctAnswer.id), '" id="comment-modDate-', soy.$$escapeHtmlAttribute(opt_data.correctAnswer.id), '" class="font-color-meta-light">', soy.$$escapeHtml(((opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : '') ? require('moment')((parseFloat((opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : '')) ? parseFloat((opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : '')) : (opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : ''))).locale(_jive_locale.toLowerCase().replace('_','-')).format('ll') : '')), ' ', soy.$$escapeHtml(((opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : '') ? require('moment')((parseFloat((opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : '')) ? parseFloat((opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : '')) : (opt_data.correctAnswer.updated ? require('jive/model/date').DateUtil.parseISODateTime(opt_data.correctAnswer.updated) : ''))).locale(_jive_locale.toLowerCase().replace('_','-')).format('LT') : '')), '</a></span></div><div class="j-mark-correct"><div class="j-marked-correct"><span class="jive-icon-glyph icon-star3"></span>', soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k33b5'),[])), '</div></div><div id="comment-body-', soy.$$escapeHtmlAttribute(opt_data.correctAnswer.id), '">', soy.$$filterNoAutoescape(opt_data.correctAnswer.content.text), '</div><div class="j-context-link"><a href="#comment-', soy.$$escapeHtmlAttribute(opt_data.correctAnswer.id), '" class="js-context-link">', soy.$$escapeHtml(jive.i18n.i18nText(jive.i18n.getMsg('k1f22'),[])), '</a></div></div></div>');
  }
  return opt_sb ? '' : output.toString();
};
