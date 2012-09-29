#include "mainwindow.h"
#include <QtGui>
#include "windows.h"
#include "mmsystem.h"
#include "winuser.h"

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    // store number of devices
    numDevs = mixerGetNumDevs();

    QVBoxLayout *layout = new QVBoxLayout;
    signalMapper = new QSignalMapper(this);
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(clicked(int)));

    QStringList mixers = getMixers();
    for (int i = 0; i < numDevs; i++) {
        QPushButton *button = new QPushButton(mixers.at(i));
        button->setCheckable(true);
        buttons << button;
        layout->addWidget(button);
        signalMapper->setMapping(button, i);
        connect(button, SIGNAL(clicked()), signalMapper, SLOT(map()));
    }

    setWindowTitle("SoundMute (F7)");
    RegisterHotKey(winId(), 0, 0, VK_F7);
    setLayout(layout);

    // select first
    clicked(0);
}

bool MainWindow::winEvent(MSG *message, long *result)
{
    if (message->message == WM_HOTKEY) {
        hotkey();

        // message handled
        return true;
    }
    // message not yet handled
    return false;
}

void MainWindow::hotkey()
{
    if (index < numDevs-1) {
        clicked(index+1);
    } else {
        clicked(0);
    }
}

void MainWindow::clicked(int index) {
    if (index >= 0 && index < numDevs) {
        this->index = index;
        for (int i = 0; i < numDevs; i++) {
            if (i != index) {
                muteMixer(i, 1);
                buttons.at(i)->setChecked(false);
            }
        }
        muteMixer(index,0);
        buttons.at(index)->setChecked(true);
    }
}

void MainWindow::muteMixer(UINT uMxId, LONG fValue)
{
    // open mixer
    HMIXER phmx;
    if (mixerOpen(&phmx, uMxId, NULL, NULL, MIXER_OBJECTF_MIXER) == MMSYSERR_NOERROR) {
        // get line id
        MIXERLINE mxl;
        mxl.cbStruct = sizeof(MIXERLINE);
        mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_SPEAKERS;
        if (mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(phmx),
            &mxl,
            MIXER_OBJECTF_HMIXER |
            MIXER_GETLINEINFOF_COMPONENTTYPE) == MMSYSERR_NOERROR) {

            // get master mute control
            MIXERCONTROL mxc;
            MIXERLINECONTROLS mxlc;
            mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
            mxlc.dwLineID = mxl.dwLineID;
            mxlc.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
            mxlc.cControls = 1;
            mxlc.cbmxctrl = sizeof(MIXERCONTROL);
            mxlc.pamxctrl = &mxc;
            if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(phmx),
                                       &mxlc,
                                       MIXER_OBJECTF_HMIXER |
                                       MIXER_GETLINECONTROLSF_ONEBYTYPE) == MMSYSERR_NOERROR) {
                // mute mixer
                MIXERCONTROLDETAILS_BOOLEAN mxcdMute;
                mxcdMute.fValue = fValue;
                MIXERCONTROLDETAILS mxcd;
                mxcd.cbStruct = sizeof(mxcd);
                mxcd.dwControlID = mxc.dwControlID;
                mxcd.cChannels = 1;
                mxcd.cMultipleItems = 0;
                mxcd.cbDetails = sizeof(mxcdMute);
                mxcd.paDetails = &mxcdMute;
                mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(phmx),
                                             &mxcd,
                                             MIXER_OBJECTF_HMIXER |
                                             MIXER_SETCONTROLDETAILSF_VALUE);

            }

        }

        // close mixer
        mixerClose(phmx);
    }
}

QStringList MainWindow::getMixers()
{
    MIXERCAPS mixercaps;
    QStringList names;

    for (int i = 0; i < numDevs; i++) {
        mixerGetDevCaps(i, &mixercaps, sizeof(mixercaps));
        names << QString::fromWCharArray(mixercaps.szPname);
    }

    return names;
}
