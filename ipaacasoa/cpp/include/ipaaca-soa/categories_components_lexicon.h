/*
 * Common Categories and Components macros for ipaaca C++ projects
 *
 * ported from ipaaca1
 */

#ifndef CATEGORIES_COMPONENTS_LEXICON_H_
#define CATEGORIES_COMPONENTS_LEXICON_H_

#include <string>

//
// Categories
//
//  please indicate sub-categories (e.g. component-specific data) and/or add descriptions

/// execution request for MURML string
const std::string CAT__MURML_DATA = "murmldata";
const std::string CAT__MURML_FILE = "murmlfile";

/// debugging (e.g. producing faults at will; messaging; logging)
const std::string CAT__DEBUGGING = "debugging";

/// Result from speech recognition
const std::string CAT__ASR_RESULT = "asrresult";

/// STOP (Interrupt current behaviors)
const std::string CAT__STOP_BEHAVIOR = "stopbehavior";

/// request to Mary TTS client
const std::string CAT__MARYTTSREQUEST = "maryttsrequest";
/// answer from Mary TTS client
const std::string CAT__MARYTTSREPLY = "maryttsreply";
/// TTS info IU (is speech in progress? when will it end? etc.)
const std::string CAT__MARYTTSINFO = "maryttsinfo";

/// WIImote controller input
const std::string CAT__CONTROLLER_WII = "wiiinput";


/// (OgreCalendar:) sending updates to GUI calendar
const std::string CAT__CALENDAR_CONTROL = "calendarcontrol";
/// (OgreCanvas:) sending updates to generic Canvas component
const std::string CAT__CANVAS_CONTROL = "canvascontrol";
/// (OgreVideoCall:) sending control commands to the video call component
const std::string CAT__VIDEOCALL_CONTROL = "videocallcontrol";
/// (OgreTrafficLight:) sending updates to GUI traffic light
const std::string CAT__TRAFFIC_LIGHT_CONTROL = "trafficlightcontrol";


/// Data from 2012 Dialogue Demonstrator XCF bridge
const std::string CAT__XCF_BRIDGE = "xcfbridge";

/// Data from old (2010) Wizard-of-Oz GUI
const std::string CAT__GUI_DATA = "guidata";


/// SCH3 Vince (from Schnick-Schnack-Schnuck / Rock-Paper-Scissors)
const std::string CAT__SCH3_GESTURE = "sch3gesture";
const std::string CAT__SCH3_OPTIONS = "sch3options";


//
// UNDOCUMENTED CATEGORIES
//
//    --- YET TO BE DOCUMENTED ! ---
//
//    Please no not add new categories to this section!
//
const std::string CAT__PERCEPTION_MESSAGE = "perceptionmessage";
const std::string CAT__PERCEPTION_WRISTS = "perceptionwristspositions";
const std::string CAT__PERCEPTION_HEIGHT = "perceptionheight";
const std::string CAT__PERCEPTION_HEIGHT_REQ = "perceptionheightreq";
const std::string CAT__PERCEPTION_CENTEROFMASS = "perceptioncenterofmass";
const std::string CAT__PERCEPTION_CENTEROFMASS_REQ = "perceptioncenterofmassreq";
const std::string CAT__CONTROLLER_COGNITIVEPROCESS = "controllercognitiveprocess";
const std::string CAT__GENERATION_WRISTS = "generationwrists";
const std::string CAT__GENERATION_MP = "generationmp";
const std::string CAT__GENERATION_GUIDING_SEQUENCE = "generationgss";
const std::string CAT__IK_GENERATION_WRISTS = "ikgenerationwrists";
const std::string CAT__SELF_COLLISION_SETTINGS = "selfcollisionsettings";
const std::string CAT__MOTORBODY_NOTIFICATION = "motorbodynotification";
const std::string CAT__SMKM_MOTOR_KNOWLEDGE = "smkmmotorknowledge";
const std::string CAT__SMKM_PARAMETERS = "smkmparameters";
const std::string CAT__SMKM_MENU = "smkmmenu";
const std::string CAT__SMKM_PARAMETER_ASSIGNMENT = "smkmparameterassignment";
const std::string CAT__SMKM_USER_MESSAGE = "smkmusermessage";
const std::string CAT__SECONDARY_BEHAVIOR = "secondarybehavior";
const std::string CAT__SMKM_WAVING = "smkmuserwaving";
const std::string CAT__SMKM_RESET = "resetforsmkmmodel";
const std::string CAT__SMKM_BRAIN_MODE = "smkmbrainmode";
const std::string CAT__SMKM_DROPPED_MP_HYPOS = "droppedmphyposnames";
const std::string CAT__DM_ASR_LABEL = "dialogmanagerasrlabel";
const std::string CAT__DM_SMKM_BRAIN_MODE = "dialogmanagersmkmbrainmode";
const std::string CAT__VISUALIZE_OBSERVED_WRISTS = "visualizeobservedwrists";
const std::string CAT__HYPOTHESES_VISUALIZATION = "hypothesesvisualization";
const std::string CAT__TRAJECTORY_VISUALIZATION = "trajectoryvisualization";
const std::string CAT__BELIEF_PROBABILITY_VISUALIZATION = "beliefprobabilityvisualization";
const std::string CAT__BELIEF_PROBABILITY_DISTRIBUTION = "beliefprobabilitydistribution";
const std::string CAT__RESET_PROBABILITY_DISTRIBUTION = "resetprobabilitydistribution";
const std::string CAT__ACE_FIGURE_TFM_MATRIX = "acetfmmatrix";
const std::string CAT__ACE_FIGURE_TFM_MATRIX_REQ = "acetfmmatrixreq";
const std::string CAT__SHOW_IMAGE = "showimage";
const std::string CAT__FOCUS_ON_IMAGE = "focusonimage";
const std::string CAT__GENERATE_GESTURE = "generategesture";
const std::string CAT__DELETE_GESTURE = "deletegesture";
const std::string CAT__CHANGE_OBSERVATION_FREQUENCY = "changeobservationfrequency";
const std::string CAT__DM_NUMBER_OF_GESTURES_REQ = "reqnumberofgestures";
const std::string CAT__DM_NUMBER_OF_GESTURES = "reqnumberofgestures";
const std::string CAT__TEXT_TO_SPEECH = "texttospeech";
const std::string CAT__OGRE_MODIFICATION_COMMAND = "ogremodificationcmd";
const std::string CAT__GESTURE_RESULT = "gestureresult";
const std::string CAT__DIALOGMANAGER_GESTURE_LEARNING = "gesturelearning";
const std::string CAT__RETRACT = "postureretraction";
const std::string CAT__BELIEF_PROBABILITY = "beliefprobability";
// End of UNDOCUMENTED CATEGORIES
// Please do not add to this section, add new categories (with /// docstring)
// to the paragraph preceding this undocumented section





// (Old definitions from ipaaca1:)
//
// Well-known Component names
//
const std::string COM__MARYTTS = "MaryTTS";
const std::string COM__WII_CONTROLLER = "WiiController";
const std::string COM__TEXT_INPUT = "TextInput";
const std::string COM__CSV_SENDER = "CSVSender";
const std::string COM__OGRE_CALENDAR = "OgreCalendar";
const std::string COM__OGRE_TRAFFIC_LIGHT = "OgreTrafficLight";
const std::string COM__SHARED_MOTOR_KNOWLEDGE_MODEL = "SharedMotorKnowledgeModel";
const std::string COM__DIALOG_MANAGER = "DialogManager_CPP";
const std::string COM__SCH3_DIALOGMANAGER = "SCH3DialogManager";
const std::string COM__SCH3_GESTUREPERFORMER = "SCH3GesturePerformer";


#endif /* CATEGORIES_COMPONENTS_LEXICON_H_ */
