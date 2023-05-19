/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WTextArea.h>
#include <Wt/WGridLayout.h>
#include <Wt/WToolBar.h>
#include <Wt/WJavaScript.h>
#include <Wt/WBorderLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WVBoxLayout.h>

#include "CodeEditor.h"
#include "DebugInspector.h"
#include "parser.h"
#include "vm.h"

 /*
  * A simple hello world application class which demonstrates how to react
  * to events, read input, and give feed-back.
  */
class HelloApplication : public Wt::WApplication
{
public:
    HelloApplication(const Wt::WEnvironment& env);

private:
    Wt::WLineEdit* nameEdit_;
    Wt::WText* greeting_;

    Wt::WToolBar* toolbar_;
//    Wt::WTextArea* codeTextArea_;
    Wt::WTextArea* compileOutputTextArea_;
    DebugInspector* debugInspector_;
    CodeEditor* codeEditor_;

    Parser parser;
    ParseTrace parse_trace;
    Bytecode bytecode;
    VM vm;

    void compile();
    void execute();
    void greet();
    Wt::JSlot scrolldown;
};

/*
 * The env argument contains information about the new session, and
 * the initial request. It must be passed to the WApplication
 * constructor so it is typically also an argument for your custom
 * application constructor.
*/
HelloApplication::HelloApplication(const Wt::WEnvironment& env)
    : WApplication(env)
{
    
//    WApplication::instance()->require("/apputils.js");

    setTitle("Hello world");                            // application title
    setCssTheme("polished");
    useStyleSheet("ants.css");

    std::unique_ptr<Wt::WToolBar> up_toolbar_ = std::make_unique<Wt::WToolBar>();
    std::unique_ptr<Wt::WPushButton> up_compileBtn_ = std::make_unique<Wt::WPushButton>("Compile");
    up_compileBtn_->clicked().connect(this, &HelloApplication::compile);
    std::unique_ptr<Wt::WPushButton> up_executeBtn_ = std::make_unique<Wt::WPushButton>("Execute");
    up_executeBtn_->clicked().connect(this, &HelloApplication::execute);
//    std::unique_ptr<Wt::WTextArea> up_codeTextArea_ = std::make_unique<Wt::WTextArea>();
//    up_codeTextArea_->setFocus();
    std::unique_ptr<Wt::WTextArea> up_compileOutputTextArea_ = std::make_unique<Wt::WTextArea>();
    up_compileOutputTextArea_->setReadOnly(true);
    std::unique_ptr<DebugInspector> up_debugInspector_ = std::make_unique<DebugInspector>();

    scrolldown.setJavaScript("setCaretPosition(" + up_compileOutputTextArea_->id() + ")");
    up_compileOutputTextArea_->changed().connect(scrolldown);

    //=== Layouts ===============================
    // Set up the main layout
    std::unique_ptr<Wt::WBorderLayout> up_main_layout = std::make_unique<Wt::WBorderLayout>();
    toolbar_ = up_main_layout->addWidget(std::move(up_toolbar_), Wt::LayoutPosition::North);

    // Arrange the center layout
    std::unique_ptr<Wt::WHBoxLayout> up_center_layout = std::make_unique<Wt::WHBoxLayout>();
    Wt::WHBoxLayout* center_layout = up_center_layout.get();

    // Arrange the left layout
    std::unique_ptr<Wt::WVBoxLayout> up_left_layout = std::make_unique<Wt::WVBoxLayout>();

    Wt::WVBoxLayout* left_layout = up_center_layout->addLayout(std::move(up_left_layout));
//!!!!!!!    debugInspector_ = up_center_layout->addWidget(std::move(up_debugInspector_));
    up_main_layout->add(std::move(up_center_layout), Wt::LayoutPosition::Center);

//    debugInspector_ = layout->addWidget(std::move(up_debugInspector_), 1, 1, 2, 1);
//!!!!!!!    codeEditor_ = left_layout->addWidget(std::make_unique<CodeEditor>(), 1);
//!!!!!!!    codeEditor_->setFocus();

//    codeTextArea_ = left_layout->addWidget(std::move(up_codeTextArea_));
 //!!!!!!   compileOutputTextArea_ = left_layout->addWidget(std::move(up_compileOutputTextArea_));
    
    toolbar_->addButton(std::move(up_compileBtn_));
    toolbar_->addButton(std::move(up_executeBtn_));

    center_layout->setResizable(0);
//!!!!!!!    left_layout->setResizable(0);
    
//    codeTextArea_->setStyleClass("nodrag_textbox");
//!!!!!!    compileOutputTextArea_->setStyleClass("nodrag_textbox");

/*    layout->setColumnResizable(0);
    layout->setRowResizable(1);
    layout->setRowStretch(1, 1);
    layout->setColumnStretch(0, 1);
*/

    root()->setLayout(std::move(up_main_layout));

//    root()->addWidget(Wt::cpp14::make_unique<Wt::WText>("Your name, please ? ")); // show some text

//    nameEdit_ = root()->addWidget(Wt::cpp14::make_unique<Wt::WLineEdit>()); // allow text input

//    auto button = root()->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Greet me."));
    // create a button
//    button->setMargin(5, Wt::Side::Left);                   // add 5 pixels margin

//    root()->addWidget(Wt::cpp14::make_unique<Wt::WBreak>());    // insert a line break
//    greeting_ = root()->addWidget(Wt::cpp14::make_unique<Wt::WText>()); // empty text


    /*
     * Connect signals with slots
     *
     * - simple Wt-way: specify object and method
     */
//    button->clicked().connect(this, &HelloApplication::greet);

    /*
     * - using an arbitrary function object, e.g. useful to bind
     *   values with std::bind() to the resulting method call
     */
//    nameEdit_->enterPressed().connect(std::bind(&HelloApplication::greet, this));

    /*
     * - using a lambda:
     */
//    button->clicked().connect([=]() {
//        std::cerr << "Hello there, " << nameEdit_->text() << std::endl;
//    });
}

void HelloApplication::compile()
{
    Wt::WString txt = compileOutputTextArea_->text();
    if (!txt.empty()) txt += "\n";
    txt += "Compilation ...";
    compileOutputTextArea_->setText(txt);

    compileOutputTextArea_->setText("");

    parser.clear();
    parse_trace.clear();

    EParseStatus ret = parser.parse(codeEditor_->text(), parse_trace, bytecode);

    if (ret == EParseStatus::PARSE_OK) {
        compileOutputTextArea_->setText("Compiled successfully\n");
    }
    else {
        Wt::WString txt = Wt::WString("Compilation errors:\n") + parse_trace.getString();
        compileOutputTextArea_->setText(txt);
    }
}

void HelloApplication::execute() {
    EExecStatus status = EExecStatus::OK_RUN;

    compileOutputTextArea_->setText("");

    vm.init();
    vm.execute(bytecode, status);
    debugInspector_->update(bytecode);

    Wt::WString txt = Wt::WString("Program execution finished: ") + exec_status_descriptions[static_cast<int>(status)] + "\n";

    compileOutputTextArea_->setText(txt);
}

void HelloApplication::greet()
{
    /*
     * Update the text, using text input into the nameEdit_ field.
     */
    greeting_->setText("Hello there, " + nameEdit_->text());
}

int main(int argc, char** argv)
{
    /*
     * Your main method may set up some shared resources, but should then
     * start the server application (FastCGI or httpd) that starts listening
     * for requests, and handles all of the application life cycles.
     *
     * The last argument to WRun specifies the function that will instantiate
     * new application objects. That function is executed when a new user surfs
     * to the Wt application, and after the library has negotiated browser
     * support. The function should return a newly instantiated application
     * object.
     */
    return Wt::WRun(argc, argv, [](const Wt::WEnvironment& env) {
        /*
         * You could read information from the environment to decide whether
         * the user has permission to start a new application
         */
        return Wt::cpp14::make_unique<HelloApplication>(env);
    });
}