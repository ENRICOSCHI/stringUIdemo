#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StringUIdemoAudioProcessorEditor::StringUIdemoAudioProcessorEditor(StringUIdemoAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    
    // --- Corde visive ---
    for (int i = 0; i < StringUIdemoAudioProcessor::numStrings; ++i)
    {
        auto* sc = stringComponents.add(new StringComponent(stringColour(i)));
        addAndMakeVisible(sc);
    }

    // --- Controlli tuning (uno per corda) ---
    for (int i = 0; i < StringUIdemoAudioProcessor::numStrings; ++i)
    {
        // Pulsante [−]
        auto* btnDown = tuningDownButtons.add(new juce::TextButton("-"));
        // quando premo il pulsante...
        btnDown->onClick = [this, i]()
            {
                int current = audioProcessor.getStringMidiNote(i);
                audioProcessor.setStringMidiNote(i, current - 1);
                updateTuningLabel(i);
            };
        addAndMakeVisible(btnDown);

        // Label con nome nota + delta semitoni
        auto* lbl = tuningLabels.add(new juce::Label());
        lbl->setJustificationType(juce::Justification::centred);
        lbl->setFont(juce::FontOptions(11.0f, juce::Font::bold));
        lbl->setColour(juce::Label::textColourId, stringColour(i));
        addAndMakeVisible(lbl);

        // Pulsante [+]
        auto* btnUp = tuningUpButtons.add(new juce::TextButton("+"));
        //quando premo il pulsante...
        btnUp->onClick = [this, i]()
            {
                int current = audioProcessor.getStringMidiNote(i);
                audioProcessor.setStringMidiNote(i, current + 1);
                updateTuningLabel(i);
            };
        addAndMakeVisible(btnUp);

    }  

    // --- Pulsante Reset ---
    resetTuningButton.setButtonText("Reset");
    resetTuningButton.onClick = [this]()
        {
            audioProcessor.resetTuning();
            updateAllTuningLabels();
        };
    addAndMakeVisible(resetTuningButton);

    // --- Label nota suonata ---
    addAndMakeVisible(notaSuonataLabel);
    notaSuonataLabel.setText("negro", juce::NotificationType::dontSendNotification);
    notaSuonataLabel.setFont(juce::FontOptions(13.0f));
    notaSuonataLabel.setColour(juce::Label::textColourId, juce::Colours::white);

	// --- Manopole ---
    for (int i = 0; i < numManopole; ++i)
    {
        // Setup Manopola
        manopolaEffetto[i].setSliderStyle(juce::Slider::Rotary);
        manopolaEffetto[i].setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        manopolaEffetto[i].setNumDecimalPlacesToDisplay(2);
        manopolaEffetto[i].setRange(0.0f, 1.0f);
        manopolaEffetto[i].setValue(0.5f);
        manopolaEffetto[i].setLookAndFeel(&stilePomello); // Usiamo lo stesso stile per tutte
        addAndMakeVisible(manopolaEffetto[i]);

        // Setup Titolo
        titoloManopolaEffetto[i].setText("Effetto " + juce::String(i + 1), juce::dontSendNotification);
        titoloManopolaEffetto[i].setJustificationType(juce::Justification::centred);
        titoloManopolaEffetto[i].setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(titoloManopolaEffetto[i]);
    }

    // Inizializza tutte le label di tuning con i valori correnti
    updateAllTuningLabels();

    setSize(750, 420);

    // Rende la finestra ridimensionabile (questo è possibile grazie ai LocalBounds settati in precedenza)
    setResizable(true, true);

    // Limiti di dimensione della finestra
    setResizeLimits(750, 420, 1200, 800);

	// Avvio il timer per controllare le interazioni Audio Thread -> UI Thread (per la MIDI)
	startTimerHz(60); // Timer che scade 60 volte al secondo (ogni ~16ms)
}

StringUIdemoAudioProcessorEditor::~StringUIdemoAudioProcessorEditor() 
{
	stopTimer(); // Ferma il timer quando l'editor viene distrutto | good practice.
}

//==============================================================================

// Override del Callback del timer come specificato in PluginEditor.h
void StringUIdemoAudioProcessorEditor::timerCallback() 
{
    // Controllo per ogni corda se la rispettiva flag è stata alzata dall'Audio Thread (processBlock)
	// Tutto tramite Polling dell'atomic <bool> uiStringWasPlucked[numStrings]
    for (int i = 0; i < StringUIdemoAudioProcessor::numStrings; ++i) 
    {
        // Check della flag, utilizzando il valore attuale e portandola a false
        if (audioProcessor.uiStringWasPlucked[i].exchange(false)) 
        {
            float position = audioProcessor.uiPluckPosition[i].load(); // Posizione del pizzico
			stringComponents.getUnchecked(i)->stringPlucked(position); // Aggiorna la visualizzazione della corda

            #pragma region Aggiornamento label nota suonata

            // Calcolo la fret della posizione relativa [0.0, 1.0]
			int fret = juce::jlimit(0, numFret, (int)(position * numFret));

			// Recupero la nota MIDI base della corda e vi sommo la fret per la nota MIDI effettiva suonata
			int midiNote = audioProcessor.getStringMidiNote(i) + fret;

            // Per il nome della nota riprendo la logica usata per esempio in HandleMouseEvent:
            // (sempre attraverso la utility di JUCE)
			juce::String nomeNota = juce::MidiMessage::getMidiNoteName(midiNote, true, true, 3);

            // Aggiorno la label di conseguenza
            notaSuonataLabel.setText("Nota: " + nomeNota + "  Tasto: " + juce::String(fret),
				juce::dontSendNotification);

            #pragma endregion
        }
    }

}

//==============================================================================

#pragma region paint UI

void StringUIdemoAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF201513));

    SetTitle(g);
    SetLineaSeparatrice(g);
    SetStrings(g);
    SetSeparationFret(g);

    // Disegno delle aree
    
    // Imposta un colore semitrasparente per evidenziare le aree
	g.setColour(juce::Colours::white.withAlpha(0.2f));

    // Disegna un bordo
    g.drawRect(areaParametriSinistra.reduced(4), 2.0f);
	g.drawRect(areaEffettiDestra.reduced(4), 2.0f);


}

#pragma endregion

#pragma region resized UI

void StringUIdemoAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

	// Sezione corde (parte inferiore, con i controlli di tuning a sinistra)
    const int stringH = 24;
    const int gap = 6;
    const int totalStrings = StringUIdemoAudioProcessor::numStrings;
    const int rightMargin = 10;

    const int stringsAreaH = totalStrings * stringH + (totalStrings - 1) * gap + 16;

    // Taglia la parte in basso per le corde
    auto bottomArea = area.removeFromBottom(stringsAreaH);

    // Salva l'area delle corde
	areaCordeSotto = bottomArea;

    // Posiziona il Pulsante Reset appena sopra l'area delle corde
    resetTuningButton.setBounds(4, bottomArea.getY() - 28, tuningPanelWidth - 8, 22);

    // Toglie gli 8 pixel di padding superiore iniziale prima di disegnare le corde
    bottomArea.removeFromTop(8);

    for (int i = 0; i < totalStrings; ++i)
    {
		// Taglia un'area per la corda i-esima
        auto row = bottomArea.removeFromTop(stringH);
		// Aggiunge un gap dopo ogni corda in modo che non si attacchino tra loro
        bottomArea.removeFromTop(gap);

        // Taglio l'area per la zona di tuning a sinistra della corda
        auto tuningRow = row.removeFromLeft(tuningPanelWidth);

		// Aggiungo un gap a destra per non far attaccare le corde al bordo
        row.removeFromRight(rightMargin);
		// Posiziono la corda i-esima nell'area rimanente
        stringComponents.getUnchecked(i)->setBounds(row);

        int btnW = 22;
        int lblW = 44;

        // Posizionamento dei controlli di tuning nella colonna a sinistra
        tuningRow.removeFromLeft(4);
        tuningDownButtons.getUnchecked(i)->setBounds(tuningRow.removeFromLeft(btnW).reduced(0, 1));

        tuningRow.removeFromLeft(2);
        tuningLabels.getUnchecked(i)->setBounds(tuningRow.removeFromLeft(lblW).reduced(0, 1));

        tuningRow.removeFromLeft(4);
        tuningUpButtons.getUnchecked(i)->setBounds(tuningRow.removeFromLeft(btnW).reduced(0, 1));
    }


	// Sezione superiore (manopola effetto + titolo)

    // Rimuove 40 pixel in alto per lasciare spazio al titolo
    area.removeFromTop(40);

    // Spazio per la Label della nota suonata
    auto notaSuonataArea = area.removeFromBottom(30);
    notaSuonataLabel.setBounds(notaSuonataArea.withSizeKeepingCentre(300, 24));
    notaSuonataLabel.setJustificationType(juce::Justification::centred); // Centriamo il testo

    // Divisione a metà della zona superiore
    auto leftParamsArea = area.removeFromLeft(area.getWidth() / 2); // Metà sinistra (pronta per il futuro)
    auto rightEffectsArea = area; // Quello che avanza è la metà destra

    // Salva le due aree superiori
    areaParametriSinistra = leftParamsArea;
	areaEffettiDestra = rightEffectsArea;


    // Calcolo dinamico della dimensione della manopola in base alla grandezza della finestra

    // Riduce l'area per non toccare i bordi visivi
    auto workArea = rightEffectsArea.reduced(10);

    // Divide la workArea a metà orizzontalmente (Riga Superiore e Riga Inferiore)
    auto topRow = workArea.removeFromTop(workArea.getHeight() / 2);
    auto bottomRow = workArea; // Quello che avanza è la riga inferiore

    // Crea un array di 4 "Celle" (Rettangoli) per ospitare le manopole
    juce::Rectangle<int> celleManopole[4];

    celleManopole[0] = topRow.removeFromLeft(topRow.getWidth() / 2); // Cella in alto a SX
    celleManopole[1] = topRow;                                       // Cella in alto a DX

    celleManopole[2] = bottomRow.removeFromLeft(bottomRow.getWidth() / 2); // Cella in basso a SX
    celleManopole[3] = bottomRow;                                          // Cella in basso a DX

    // Posiziona le manopole all'interno delle rispettive celle
    int maxKnobSize = 100;
    int minKnobSize = 65;

    for (int i = 0; i < numManopole; ++i)
    {
        // Usa la logica di adattamento dinamico
        int calculatedSize = juce::jmin(celleManopole[i].getWidth(), celleManopole[i].getHeight() - 25, maxKnobSize);
        int dynamicKnobSize = juce::jmax(minKnobSize, calculatedSize);

        // Centra il riquadro della manopola all'interno della sua cella specifica
        auto manopolaBounds = celleManopole[i].withSizeKeepingCentre(dynamicKnobSize, dynamicKnobSize);

        manopolaEffetto[i].setBounds(manopolaBounds);
        titoloManopolaEffetto[i].setBounds(manopolaBounds.getX(), manopolaBounds.getY() - 25, dynamicKnobSize, 20);
    }
}

#pragma endregion

//==============================================================================
void StringUIdemoAudioProcessorEditor::handleMouseEvent(const juce::MouseEvent& e)
{
    for (int i = 0; i < stringComponents.size(); ++i)
    {
        auto* sc = stringComponents.getUnchecked(i);

        if (sc->getBounds().contains(e.getPosition()))
        {
            float relPos = (e.position.x - (float)sc->getX()) / (float)sc->getWidth();
            relPos = juce::jlimit(0.0f, 1.0f, relPos);

            int posFret = juce::jlimit(0, numFret, (int)(relPos * numFret));

            // La nota suonata dipende dalla nota base CORRENTE (accordatura attuale)
            int baseMidi = audioProcessor.getStringMidiNote(i);
            int midiNote = baseMidi + posFret;

            if (oldPosFret != posFret || oldMidiNote != midiNote)
            {
                oldPosFret = posFret;
                oldMidiNote = midiNote;

                juce::String nomeNota = juce::MidiMessage::getMidiNoteName(midiNote, true, true, 3);
                notaSuonataLabel.setText("Nota: " + nomeNota + "  Tasto: " + juce::String(posFret),
                    juce::dontSendNotification);

                sc->stringPlucked(relPos);
                audioProcessor.pluckString(i, relPos);
                break;
            }
        }
    }
}

//==============================================================================

/// <summary>
/// Tuning label: mostra nome nota base + delta in semitoni rispetto al default.
/// Es.: "E2 (+2)" oppure "E2 (-1)" oppure "E2"
/// </summary>
/// <param name="stringIndex"></param>
void StringUIdemoAudioProcessorEditor::updateTuningLabel(int stringIndex)
{
    if (stringIndex < 0 || stringIndex >= StringUIdemoAudioProcessor::numStrings)
        return;

    int currentNote = audioProcessor.getStringMidiNote(stringIndex);
    int defaultNote = StringUIdemoAudioProcessor::defaultMidiNotes[stringIndex];
    int delta = currentNote - defaultNote;

    // Nome della nota base (senza numero di ottava nei pulsanti, con ottava nella label)
    juce::String noteName = juce::MidiMessage::getMidiNoteName(currentNote, true, true, 3);

    juce::String deltaStr;
    if (delta > 0) deltaStr = "(+" + juce::String(delta) + ")";
    else if (delta < 0) deltaStr = "(" + juce::String(delta) + ")";
    // se delta == 0 non mostriamo nulla

    juce::String labelText = noteName;
    if (deltaStr.isNotEmpty())
        labelText += " " + deltaStr;

    tuningLabels.getUnchecked(stringIndex)->setText(labelText, juce::dontSendNotification);
}

void StringUIdemoAudioProcessorEditor::updateAllTuningLabels()
{
    for (int i = 0; i < StringUIdemoAudioProcessor::numStrings; ++i)
        updateTuningLabel(i);
}

//==============================================================================
void StringUIdemoAudioProcessorEditor::SetTitle(juce::Graphics& g)
{
    g.setColour(juce::Colours::white.withAlpha(0.85f));
    g.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    g.drawText("String UI Demo - pizzica le corde!",
        0, 10, getWidth(), 30, juce::Justification::centred);
}

void StringUIdemoAudioProcessorEditor::SetLineaSeparatrice(juce::Graphics& g)
{
    auto stringsAreaY = getHeight() - (StringUIdemoAudioProcessor::numStrings * 30 + 16);
    g.setColour(juce::Colour(0xFF4D453A));
    g.drawHorizontalLine(stringsAreaY - 4, 10.0f, (float)getWidth() - 10.0f);
}

void StringUIdemoAudioProcessorEditor::SetStrings(juce::Graphics& g)
{
    // Le etichette testuali (E2, A2...) vengono mostrate dalla tuningLabel,
    // quindi qui disegniamo solo una eventuale riga di sfondo per le corde
    // (opzionale, puoi rimuoverlo)
    for (int i = 0; i < StringUIdemoAudioProcessor::numStrings; ++i)
    {
        auto* sc = stringComponents.getUnchecked(i);
        g.setColour(stringColour(i).withAlpha(0.08f));
        g.fillRect(sc->getX(), sc->getY(), sc->getWidth(), sc->getHeight());
    }
}

void StringUIdemoAudioProcessorEditor::SetSeparationFret(juce::Graphics& g)
{
    auto* sc = stringComponents.getUnchecked(0);

    float startX = (float)sc->getX();
    float width = (float)sc->getWidth();
    float fretW = width / (float)numFret;

    g.setColour(juce::Colour(0xFFA88CEC));

    for (int i = 0; i <= numFret; ++i)
    {
        float x = startX + i * fretW;
        g.fillRect((int)x, sc->getY(), 3, sc->getHeight() * (numCorde + 1));
    }
}