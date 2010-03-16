<?php
namespace Extool\Target;

class Joomla15 implements \Extool\Target\TargetInterface
{
	private $rep;
	private $files;
	private $snippets;

	public function setRepresentation(\Extool\Representation\Representation $representation)
	{
		$this->rep = $representation;
	}

	public function generate()
	{
		$this->snippets = new \Extool\Helpers\SnippetFactory();
		$this->snippets->setBasePath('targets/Joomla15/snip');
		$this->files = new \Extool\Helpers\FilePackage();

		// TODO: There's probably a better way than this
		$this->generateMySQL();
		$this->makeTableClasses();
		$this->makeViews();

		return $this->files;
	}

	private function generateMySQL()
	{
		$install_file = new \Extool\Helpers\File();
		$uninstall_file = new \Extool\Helpers\File();

		foreach ($this->rep->tables as $table) {
			$table->createDefaultKey();

			$mysql = new \Extool\Helpers\MySQL($table);
			$mysql->setName('#__' . $table->name);
			$install_file->appendContents($mysql->generateCreate() . "\n\n");
			$uninstall_file->appendContents($mysql->generateDrop() . "\n");
		}

		$this->files->addFile('admin/install.mysql.sql', $install_file);
		$this->files->addFile('admin/uninstall.mysql.sql', $uninstall_file);
	}

	private function makeTableClasses()
	{
		foreach ($this->rep->tables as $table) {
			$table_name = strtolower(str_replace(' ', '_', $table->name));

			if ($table_name != $this->rep->name) {
				$table_name = $this->rep->name . '_' . $table_name;
			}

			$tableSnip = $this->snippets->getSnippet('table');
			$tableSnip->assign('table_name', $table_name);
			$tableSnip->assign('table_uc_name', ucfirst($table_name));

			foreach ($table->fields as $field) {
				$field_snip = $this->snippets->getSnippet('table_field');
				$field_snip->assign('field', strtolower($field['name']));
				$tableSnip->add('variables', $field_snip);
			}

			$fileSnip = $this->snippets->getSnippet('code');
			$fileSnip->assign('code', $tableSnip);

			$tableFile = new \Extool\Helpers\File();
			$tableFile->setContents($fileSnip);
			$this->files->addFile("admin/tables/{$table_name}.php", $tableFile);
		}	
	}

	private function makeViews()
	{
		foreach ($this->rep->public_views as $view) {
			$codeView = new Joomla15\helpers\view($view, $this->rep->name);
			$clean_view_name = strtolower(str_replace(' ', '_', $view->name));

			$path = "site/views/" . $clean_view_name . '/view.html.php';
			$this->files->addFile($path, $codeView->makeViewClass());

			$path = "site/views/" . $clean_view_name . '/tmpl/default.php';
			$this->files->addFile($path, $codeView->makeViewTmpl());
		}

		foreach ($this->rep->admin_views as $view) {
			$codeView = new Joomla15\helpers\view($view, $this->rep->name, true);

			$path = "admin/views/" . $clean_view_name . '/view.html.php';
			$this->files->addFile($path, $codeView->makeViewClass());

			$path = "admin/views/" . $clean_view_name . '/tmpl/default.php';
			$this->files->addFile($path, $codeView->makeViewTmpl());
		}
	}
}
