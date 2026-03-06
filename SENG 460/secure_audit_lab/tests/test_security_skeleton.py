import pytest

@pytest.fixture
def app_client():
    import app as app_module
    app_module.app.config["TESTING"] = True
    client = app_module.app.test_client()
    yield client

def test_homepage_loads(app_client):
    r = app_client.get("/")
    assert r.status_code == 200

# TODO (students):
# Add at least 4 regression tests after patching:
# - SQL injection login bypass fails
# - IDOR delete/edit blocked (ownership check)
# - CSRF token required for POST
# - Export path cannot escape data directory
