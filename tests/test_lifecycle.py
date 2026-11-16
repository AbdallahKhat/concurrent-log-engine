def test_boots_and_stops(engine):
    assert engine["logger"].poll() is None
    assert engine["ingest"].poll() is None
